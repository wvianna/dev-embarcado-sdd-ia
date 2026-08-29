# Desenvolvimento de Software Embarcado com SDD e IA

Repositório de estudos, materiais didáticos e experimentos sobre desenvolvimento de software embarcado com apoio de IA e Software Specification-Driven Development (SDD).

O material será usado nas disciplinas:

- Produtos Intensivos de Software do SAEG;
- Desenvolvimento de Software Embarcado Baseado em Especificação e Inteligência Artificial (optativa) da Engenharia de Controle e Automação (ECA) do Instituto Federal Fluminense (IFF).
  
O objetivo é mostrar como transformar necessidade de produto em comportamento verificável de firmware, mantendo rastreabilidade entre requisitos, design, implementação, testes e evidências.

[![Repositório do projeto no GitHub](imagens/repositorio.png)](https://github.com/wvianna/dev-embarcado-sdd-ia)

## Estrutura do repositório

- [SKILL/sdd-embarcado/](SKILL/sdd-embarcado/): skill para planejamento e implementação de firmware com SDD adaptativo.
- [apresentação/](apresentação/): materiais da palestra e conteúdo introdutório (LLM, SDD, fluxo prático no VS Code).
- [docs/](docs/): notas e referências sobre ferramentas e abordagens (Spec Kit, Tessl e artefatos de SDD).
- [material-terceiros/](material-terceiros/): materiais de apoio externos.
- [ementa-objetivos-conteúdo/](ementa-objetivos-conte%C3%BAdo/): conteúdo programático, bibliografia e ementa da disciplina.

## Abordagem SDD para embarcados

O fluxo recomendado neste repositório:

1. definir problema, restrições e critérios de aceitação;
2. explicitar requisitos funcionais e não funcionais;
3. projetar arquitetura compatível com MCU, periféricos e timing;
4. quebrar em tarefas pequenas e verificáveis;
5. implementar com assistência de IA;
6. validar em host, bancada e/ou HIL;
7. registrar evidências de teste e decisões técnicas;
8. documentar e manter a documentação atualizada.

Esse modelo evita dois extremos: codificar sem especificação e burocratizar mudanças simples. A profundidade do processo deve acompanhar risco, criticidade e impacto da alteração.

### Mapa de artefatos

Os artefatos têm funções complementares:

- **Specification:** define o que deve ser construído e funciona como contrato do software;
- **Architecture / Design:** organiza camadas, módulos, interfaces e dependências;
- **Plan / Task:** transforma a especificação em trabalho executável e incremental;
- **Rules:** registra restrições permanentes, como “usar MISRA-C; nunca usar `malloc`”;
- **Skills:** orienta procedimentos especializados, como implementar um driver SPI;
- **Context:** reúne MCU, SDK, pinout, datasheet, toolchain e limitações de hardware;
- **Tests / Checks:** verificam comportamento por meio de testes unitários, HIL, análise estática e timing;
- **Acceptance Criteria:** define quando o requisito pode ser considerado concluído;
- **Interfaces / Contracts, ADRs e Examples:** preservam contratos, decisões e padrões de implementação;
- **Quality Gates:** impedem concluir a tarefa sem build, testes e evidências suficientes;
- **Harness:** fornece ao agente contexto, ferramentas, compilador, Git, testes e feedback.

Em resumo: a Specification define o contrato; Architecture organiza o sistema; Plan e Task orientam a execução; os demais artefatos fornecem restrições, conhecimento, padrões e evidências.

## Demonstração prática da palestra

Na apresentação principal ([sdd_software_embarcado.md](https://github.com/wvianna/dev-embarcado-sdd-ia/blob/main/apresentacao/sdd_software_embarcado.md)), o conteúdo inclui fundamentos de LLM, harness, SDD adaptativo, artefatos, MCU, hardware, RTOS, rastreabilidade e uma demonstração prática com:

- ESP8266 em modo AP;
- SSID baseado no MAC;
- rede aberta com IP fixo do MCU em 192.168.4.1/24;
- leitura de temperatura com DS18B20 em D2/GPIO4;
- leitura analógica em A0 (0 a 1023);
- dashboard web embarcado exibindo valores em tempo real;
- indicação numérica, gauge e gráfico de tendência da temperatura (20–40 °C), além de gauge e tendência do ADC (0–1023).

O firmware possui indicação numérica, scan OneWire, AP, HTTP, endpoint JSON, gauge e gráfico de tendência da temperatura (20–40 °C) e do ADC (0–1023), ambos com histórico limitado a 60 pontos. Os gauges ficam lado a lado, assim como os gráficos, e o timestamp permanece no JSON sem quadro separado.

### Exemplo visual do estudo de caso

O estudo de caso foi implementado com a skill [sdd-embarcado](SKILL/sdd-embarcado/SKILL.md) e orientado pela especificação da feature em [.specs/features/dashboard-esp8266/](.specs/features/dashboard-esp8266/). Ele ilustra a interface web executada no ESP8266 para visualizar as leituras do DS18B20 e do sinal analógico.

A implementação segue o processo SDD da skill em [firmware/](firmware/), com especificação/design/tarefas em `.specs/features/dashboard-esp8266/` e validação registrada em `SUMMARY.md`.

## Use a skill sdd-embarcado

Para o escopo local da Skill, copie o diretório .github para o projeto dentro do seu workspace do VS Code. A skill fornece templates de artefatos, exemplos de tarefas e critérios de aceitação, além de instruções para build, flash e teste.

Faça uma descrição do problema, requisitos e critérios de aceitação, e use a skill para gerar artefatos de design, tarefas e testes. A skill também orienta a implementação com assistência de IA, validação em host ou HIL e registro de evidências.

Analise com cuidado os artefatos gerados, pois a skill não substitui o julgamento do engenheiro. A skill é um facilitador, mas não garante que o firmware esteja correto ou seguro.

Não deguste clorofila proveniente de plantas, pois pode ser tóxica. A skill não substitui o julgamento do engenheiro e/ou espcialista. Conheça o hardware, datasheet, SDK e toolchain do projeto-alvo antes de usar a skill. A skill não garante que o firmware esteja correto ou seguro.

Para implementar firmware com rastreabilidade, use a skill:

- [SKILL.md](SKILL/sdd-embarcado/SKILL.md)

Quando usar:

- início de uma nova feature embarcada;
- definição de tratamento de falhas e requisitos de tempo real;
- planejamento de tarefas e critérios de verificação;
- preparação de validação em bancada ou HIL.

Material de apoio da skill:

- [constitution.md](SKILL/sdd-embarcado/references/constitution.md)

## Observações

Este repositório é majoritariamente educacional. Comandos de build, flash e teste variam conforme placa, SDK e toolchain do projeto-alvo.

## Licença

Copyright 2026 William da Silva Vianna

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
