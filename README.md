# Desenvolvimento de Software Embarcado com SDD e IA

Repositório de estudos, materiais didáticos e experimentos sobre desenvolvimento de software embarcado com apoio de IA e Software Specification-Driven Development (SDD).

O material será usado nas disciplinas:
- Produtos Intensivos de Software do SAEG;
- Desenvolvimento de Software Embarcado Baseado em Especificação e Inteligência Artificial (optativa) da Engenharia de Controle e Automação (ECA) do Instituto Federal Fluminense (IFF).
  
O objetivo é mostrar como transformar necessidade de produto em comportamento verificável de firmware, mantendo rastreabilidade entre requisitos, design, implementação, testes e evidências.

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
7. registrar evidências de teste e decisões técnicas.

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

Na apresentação principal ([apresentação/palestra_sdd_software_embarcado.md](apresentação/palestra_sdd_software_embarcado.md)), o conteúdo inclui fundamentos de LLM, harness, SDD adaptativo, artefatos, MCU, hardware, RTOS, rastreabilidade e uma demonstração prática com:

- ESP8266 em modo AP;
- SSID baseado no MAC;
- rede aberta com IP fixo do MCU em 192.168.4.1/24;
- leitura de temperatura com DS18B20 em D2/GPIO4;
- leitura analógica em A0 (0 a 1023);
- dashboard web embarcado exibindo valores em tempo real.

## Use a skill sdd-embarcado

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
