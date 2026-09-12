#!/usr/bin/env python3
"""Bateria de proteção em bancada — firmware env `bancada` (FR-030 / CA-008…CA-016, CA-024).

Uso:
    python3 firmware/tools/bench_injection_test.py [--port /dev/ttyUSB0] [--baud 115200]

Protocolo do console (design §4.6): comandos em linha; resposta em uma linha
`OK …`, `ERR …` ou `JSON {…}`. A injeção substitui a leitura física do DS18B20
até o comando REAL (e é volátil: um REBOOT volta ao sensor real).

Cobre, no firmware real (com segurança elétrica — a carga pode ficar desconectada):
- fronteira 79,90 °C (mantém) / 80,00 °C (corte + latch na mesma avaliação);
- recusa de ON com latch/falha/ausência, recusa de rearme quente;
- persistência do latch ao esfriar e rearme manual com leitura válida;
- retomada após falha sem religamento automático (DEC-04);
- métricas de amostragem (CA-004) e reset volátil do latch (CA-018).
"""

import argparse
import json
import sys
import time

try:
    import serial
except ImportError:  # pragma: no cover - ambiente sem pyserial
    print("ERRO: pyserial não instalado. Instale com: pip install pyserial", file=sys.stderr)
    sys.exit(2)

RESPONSE_PREFIXES = ("OK", "ERR", "JSON")
SAMPLE_WINDOW_MS = (1050, 1350)  # CA-004: 1200 ms ± 150 ms (DEC-07)


class BenchError(RuntimeError):
    """Falha de comunicação com o console de bancada."""


class Bench:
    def __init__(self, port: str, baud: int, timeout: float = 4.0):
        self.timeout = timeout
        self.ser = serial.Serial(port, baud, timeout=0.1)
        try:  # evita reset involuntário por DTR/RTS em alguns adaptadores
            self.ser.setDTR(False)
            self.ser.setRTS(False)
        except OSError:
            pass
        time.sleep(2.5)  # boot do firmware após abrir a porta
        self.ser.reset_input_buffer()

    def close(self) -> None:
        self.ser.close()

    def _read_line(self, timeout: float) -> str:
        deadline = time.time() + timeout
        buf = bytearray()
        while time.time() < deadline:
            byte = self.ser.read(1)
            if not byte:
                continue
            if byte in (b"\r", b"\n"):
                if buf:
                    return buf.decode("utf-8", "replace")
            else:
                buf += byte
        raise BenchError("timeout de leitura serial")

    def command(self, line: str, timeout=None) -> str:
        """Envia um comando e devolve a primeira resposta OK/ERR/JSON."""
        self.ser.write((line + "\n").encode("ascii"))
        deadline = time.time() + (timeout or self.timeout)
        while time.time() < deadline:
            try:
                resp = self._read_line(deadline - time.time())
            except BenchError as exc:
                raise BenchError(f"sem resposta para {line!r}") from exc
            if resp.startswith(RESPONSE_PREFIXES):
                return resp
        raise BenchError(f"sem resposta para {line!r}")

    def state(self) -> dict:
        resp = self.command("STATE")
        if not resp.startswith("JSON "):
            raise BenchError(f"STATE inesperado: {resp!r}")
        return json.loads(resp[5:])

    def settle(self, seconds: float = 1.5) -> dict:
        """Aguarda pelo menos um ciclo de amostragem e devolve o estado."""
        time.sleep(seconds)
        return self.state()


def run(bench: Bench) -> list:
    results: list = []

    def check(name: str, ok: bool, detail: str = "") -> None:
        results.append((name, bool(ok), detail))
        status = "PASS" if ok else "FAIL"
        extra = f"  [{detail}]" if detail else ""
        print(f"{status}  {name}{extra}")

    b = bench

    # Saneamento inicial: garante carga desligada e trava liberada (se possível).
    b.command("OFF")
    b.command("TEMP 2500")
    b.settle()
    b.command("REARM")  # se não houver latch, responde ERR not_latched (aceitável)
    b.command("OFF")

    # 1) Injeção válida, carga desligada, sem bloqueio.
    st = b.settle()
    check(
        "injeção válida aplicada (25,00 °C) com carga desligada",
        st["valid"] and st["temp"] == 2500 and st["pwm"] == 0 and st["block"] == "none",
        f"temp={st['temp']} pwm={st['pwm']} block={st['block']}",
    )

    # 2) ON efetiva PWM 1023.
    resp = b.command("ON")
    check("ON efetiva PWM 1023", resp.startswith("OK ON") and "pwm=1023" in resp, resp)

    # 3) Fronteira inferior: 79,90 °C mantém o aquecimento, sem alarme/latch.
    b.command("TEMP 7990")
    st = b.settle()
    check(
        "fronteira 79,90 °C mantém aquecimento (sem alarme/latch)",
        st["pwm"] == 1023 and not st["alarm"] and not st["latch"] and st["temp"] == 7990,
        f"pwm={st['pwm']} alarm={st['alarm']} latch={st['latch']}",
    )

    # 4) Fronteira superior: 80,00 °C corta e latcheia na mesma avaliação.
    b.command("TEMP 8000")
    st = b.settle()
    check(
        "80,00 °C corta (PWM 0) + latch + alarme na mesma avaliação",
        st["pwm"] == 0 and st["latch"] and st["alarm"] and st["block"] == "latched",
        f"pwm={st['pwm']} latch={st['latch']} alarm={st['alarm']} block={st['block']}",
    )

    # 5) ON recusado com latch ativo.
    resp = b.command("ON")
    check("ON recusado com latch ativo (reason=latched)", resp.startswith("ERR ON") and "latched" in resp, resp)

    # 6) Rearme recusado ainda quente (85,00 °C).
    b.command("TEMP 8500")
    b.settle()
    resp = b.command("REARM")
    st = b.settle()
    check(
        "rearme recusado a 85,00 °C (reason=temp_high) e latch mantido",
        resp.startswith("ERR REARM") and "temp_high" in resp and st["latch"],
        resp,
    )

    # 7) Latch persiste após esfriar; buzzer para (alarm=false).
    b.command("TEMP 5000")
    st = b.settle()
    check(
        "latch persiste após esfriar (50,00 °C); buzzer cessa",
        st["latch"] and not st["alarm"] and st["pwm"] == 0,
        f"latch={st['latch']} alarm={st['alarm']} pwm={st['pwm']}",
    )

    # 8) Rearme aceito com leitura válida < 80 °C; carga NÃO religa sozinha.
    resp = b.command("REARM")
    st = b.settle()
    check(
        "rearme aceito; carga permanece desligada (DEC-01)",
        resp.startswith("OK REARM") and not st["latch"] and st["pwm"] == 0,
        resp,
    )

    # 9) Novo ON é obrigatório após o rearme.
    resp = b.command("ON")
    check("novo ON aceito após rearme (PWM 1023)", resp.startswith("OK ON") and "pwm=1023" in resp, resp)

    # 10) Corte com a carga ligada (mesma avaliação).
    b.command("TEMP 8000")
    st = b.settle()
    check(
        "corte com carga ligada em 80,00 °C (PWM 0 + latch)",
        st["pwm"] == 0 and st["latch"],
        f"pwm={st['pwm']} latch={st['latch']}",
    )
    b.command("TEMP 5000")
    b.settle()
    b.command("REARM")

    # 11) Falha de leitura: corte imediato, estado explícito, ON recusado.
    b.command("FAULT")
    st = b.settle()
    resp = b.command("ON")
    check(
        "falha de leitura corta (PWM 0), expõe block=invalid_reading e recusa ON",
        (not st["valid"]) and st["pwm"] == 0 and st["block"] == "invalid_reading" and resp.startswith("ERR ON"),
        f"valid={st['valid']} block={st['block']} | {resp}",
    )

    # 12) Retomada: bloqueio removido, carga continua desligada; ON volta a valer.
    b.command("TEMP 2500")
    st = b.settle()
    resp = b.command("ON")
    check(
        "retomada remove bloqueio sem religar (DEC-04); novo ON aceito",
        st["block"] == "none" and st["pwm"] == 0 and resp.startswith("OK ON"),
        f"block={st['block']} pwm={st['pwm']} | {resp}",
    )

    # 13) Janela de amostragem CA-004 (1200 ms ± 150 ms) medida no firmware.
    st = b.settle()
    lo, hi = SAMPLE_WINDOW_MS
    check(
        "amostragem dentro de 1200 ms ± 150 ms (samp_min/samp_max)",
        st["samp_min_ms"] > 0 and st["samp_min_ms"] >= lo and st["samp_max_ms"] <= hi,
        f"min={st['samp_min_ms']} max={st['samp_max_ms']} last={st['samp_ms']}",
    )

    # 14) Reset com latch ativo: latch é volátil; carga volta desligada (CA-018).
    b.command("TEMP 9000")
    st = b.settle()
    latched_before = bool(st["latch"])
    b.command("REBOOT", timeout=2.0)
    time.sleep(3.0)
    b.ser.reset_input_buffer()
    st = b.settle(1.6)
    check(
        "reset limpa o latch (volátil) e mantém a carga desligada",
        latched_before and (not st["latch"]) and st["pwm"] == 0,
        f"antes={latched_before} depois latch={st['latch']} pwm={st['pwm']}",
    )

    # Estado final: volta ao sensor físico e carga desligada.
    b.command("REAL")
    b.command("OFF")
    return results


def main() -> int:
    parser = argparse.ArgumentParser(description="Bateria de proteção em bancada (env bancada)")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="porta serial (padrão /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200, help="velocidade serial (padrão 115200)")
    args = parser.parse_args()

    bench = Bench(args.port, args.baud)
    try:
        results = run(bench)
    except (BenchError, KeyError, json.JSONDecodeError) as exc:
        print(f"ERRO de execução: {exc}", file=sys.stderr)
        return 2
    finally:
        bench.close()

    passed = sum(1 for _, ok, _ in results if ok)
    total = len(results)
    print(f"\nRESUMO: {passed}/{total} verificações PASS")
    for name, ok, detail in results:
        if not ok:
            print(f"  FALHOU: {name}" + (f"  [{detail}]" if detail else ""))
    return 0 if passed == total else 1


if __name__ == "__main__":
    sys.exit(main())
