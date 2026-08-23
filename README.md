# Desenvolvimento de Software Embarcado com SDD e IA

Repositório de estudos, materiais didáticos e experimentos sobre desenvolvimento de software embarcado em microcontroladores com apoio de inteligência artificial e **Software Specification-Driven Development (SDD)**.

O projeto investiga como especificações, critérios de aceite, testes e documentação podem orientar agentes de IA na criação e manutenção de firmware, mantendo o controle humano sobre requisitos, decisões técnicas e validação em hardware.

## Conteúdo

- [`sdd/`](sdd/): skill e materiais sobre planejamento orientado a especificações, incluindo fluxo adaptativo para projetos embarcados.
- [`docs/`](docs/): requisitos, referências e anotações sobre ferramentas e abordagens de SDD, como Spec Kit e Tessl.
- [`apresentação/`](apresentação/): textos e materiais de apresentação sobre desenvolvimento de software com IA.
- [`slides-Rogerio/`](slides-Rogerio/): apresentações e conteúdos complementares.
- [`conteúdo e bibliografia.txt`](conteúdo%20e%20bibliografia.txt): conteúdo programático e referências de estudo.
- [`ementa-objetivos-conteúdo - desenvolvimento de software embarcadao com auxílio de ia - optativa.pdf`](ementa-objetivos-conteúdo%20-%20desenvolvimento%20de%20software%20embarcadao%20com%20auxílio%20de%20ia%20-%20optativa.pdf): ementa da disciplina.
- [`.github/skills/sdd-embarcado/`](.github/skills/sdd-embarcado/): skill compartilhável para especificar, implementar e validar software embarcado.

## Abordagem

O material combina:

1. levantamento de requisitos e critérios de aceite;
2. decisões de arquitetura e restrições do microcontrolador;
3. planejamento incremental conforme o tamanho e o risco da mudança;
4. implementação assistida por IA;
5. compilação, testes e validação em host, simulador, bancada ou HIL;
6. documentação, rastreabilidade e handoff para continuidade do trabalho.

A proposta é evitar tanto a codificação sem especificação quanto processos excessivamente burocráticos para alterações pequenas. O nível de formalidade deve acompanhar a complexidade, o risco e a necessidade de validação física.

## Como usar

Comece pelos requisitos e pela ementa em [`docs/`](docs/) e [`conteúdo e bibliografia.txt`](conteúdo%20e%20bibliografia.txt). Para aplicar o método em um projeto de firmware, consulte a skill [`sdd-embarcado`](.github/skills/sdd-embarcado/SKILL.md) e copie o modelo de constituição disponível em [`references/constitution.md`](.github/skills/sdd-embarcado/references/constitution.md) para o projeto-alvo.

Este repositório é principalmente documental e educacional. Os comandos de compilação, testes e gravação dependem do microcontrolador, da placa, do SDK e da toolchain definidos em cada projeto de firmware.
