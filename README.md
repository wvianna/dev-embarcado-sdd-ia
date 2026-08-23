# Desenvolvimento de Software Embarcado com SDD e IA

Repositório de estudos, materiais didáticos e experimentos sobre desenvolvimento de software embarcado com apoio de IA e Software Specification-Driven Development (SDD).

O objetivo é mostrar como transformar necessidade de produto em comportamento verificável de firmware, mantendo rastreabilidade entre requisitos, design, implementação, testes e evidências.

## Estrutura do repositório

- [.github/skills/sdd-embarcado/](.github/skills/sdd-embarcado/): skill para planejamento e implementação de firmware com SDD adaptativo.
- [apresentação/](apresentação/): materiais da palestra e conteúdo introdutório (LLM, SDD, fluxo prático no VS Code).
- [docs/](docs/): notas e referências sobre ferramentas e abordagens (Spec Kit, Tessl etc.).
- [material-terceiros/](material-terceiros/): materiais de apoio externos.
- [conteúdo e bibliografia.txt](conteúdo%20e%20bibliografia.txt): conteúdo programático e bibliografia base.
- [ementa-objetivos-conteúdo - desenvolvimento de software embarcadao com auxílio de ia - optativa.pdf](ementa-objetivos-conteúdo%20-%20desenvolvimento%20de%20software%20embarcadao%20com%20auxílio%20de%20ia%20-%20optativa.pdf): ementa da disciplina.

## Abordagem SDD para embarcados

O fluxo recomendado neste repositório:

1. definir problema, restrições e critérios de aceitação;
2. explicitar requisitos funcionais e não funcionais;
3. projetar arquitetura compatível com MCU, periféricos e timing;
4. quebrar em tarefas pequenas e verificáveis;
5. implementar com assistência de IA;
6. validar em host, bancada e/ou HIL;
7. registrar evidências de teste e decisões técnicas.

Esse modelo evita dois extremos: codificar sem especificação e burocratizar mudanças simples. A profundidade do processo deve acompanhar risco, criticidade e impacto da alteração.

## Demonstração prática da palestra

Na apresentação principal ([apresentação/palestra_sdd_software_embarcado.md](apresentação/palestra_sdd_software_embarcado.md)), a demonstração prática inclui:

- ESP8266 em modo AP;
- SSID baseado no MAC;
- rede aberta com IP fixo do MCU em 192.168.4.1/24;
- leitura de temperatura com DS18B20 em D2/GPIO4;
- leitura analógica em A0 (0 a 1023);
- dashboard web embarcado exibindo valores em tempo real.

## Use a skill sdd-embarcado

Para implementar firmware com rastreabilidade, use a skill:

- [SKILL.md](.github/skills/sdd-embarcado/SKILL.md)

Quando usar:

- início de uma nova feature embarcada;
- definição de tratamento de falhas e requisitos de tempo real;
- planejamento de tarefas e critérios de verificação;
- preparação de validação em bancada ou HIL.

Material de apoio da skill:

- [constitution.md](.github/skills/sdd-embarcado/references/constitution.md)

## Observações

Este repositório é majoritariamente educacional. Comandos de build, flash e teste variam conforme placa, SDK e toolchain do projeto-alvo.
