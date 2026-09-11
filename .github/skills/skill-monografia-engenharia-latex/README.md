# Skill — Monografia de Engenharia com LaTeX, ABNT e IA

Skill para criação, revisão e evolução de monografias de graduação em Engenharia, orientada por evidências, metodologia científica, boas práticas de engenharia, normas ABNT e produção em **LaTeX pronta para compilação e geração do PDF**.

## Estrutura

```text
skill-monografia-engenharia-latex/
├── SKILL.md
├── README.md
├── rules/
│   ├── abnt.md
│   ├── academic-writing.md
│   ├── engineering.md
│   ├── citations.md
│   ├── diagrams-mermaid.md
│   ├── figures-tables.md
│   ├── methodology.md
│   ├── results.md
│   └── quality-control.md
├── workflows/
│   ├── initialize.md
│   ├── audit-workspace.md
│   ├── write-chapter.md
│   ├── review.md
│   └── finalize.md
└── templates/
    ├── MONOGRAFIA_STATUS.md
    ├── MONOGRAFIA_PLANO.md
    ├── MONOGRAFIA_EVIDENCIAS.md
    ├── MONOGRAFIA_RASTREABILIDADE.md
    └── MONOGRAFIA_PENDENCIAS.md
```

## Princípio central

A monografia deve evoluir como um sistema técnico-acadêmico:

`PROBLEMA → OBJETIVOS → FUNDAMENTAÇÃO → METODOLOGIA → PROJETO → IMPLEMENTAÇÃO → EXPERIMENTOS → DADOS → RESULTADOS → DISCUSSÃO → CONCLUSÃO`

O objetivo de 65–100 páginas de conteúdo acadêmico é uma **faixa de planejamento**, não uma justificativa para inserir conteúdo artificial, repetitivo ou irrelevante.

## Projeto LaTeX recomendado

Ao aplicar a skill a um workspace de monografia, o agente deve produzir e manter, no mínimo:

```text
monografia/
├── main.tex
├── latexmkrc
├── references.bib
├── chapters/
├── figures/
├── diagrams/
├── tables/
├── appendices/
├── annexes/
├── MONOGRAFIA_STATUS.md
├── MONOGRAFIA_PLANO.md
├── MONOGRAFIA_EVIDENCIAS.md
├── MONOGRAFIA_RASTREABILIDADE.md
└── MONOGRAFIA_PENDENCIAS.md
```

O modelo institucional da instituição de ensino tem prioridade sobre qualquer estrutura genérica. A classe LaTeX, pacote ABNT e compilador devem ser escolhidos de acordo com o modelo adotado e verificados antes da finalização.

## Compilação

Quando o projeto utilizar `latexmk`, a validação padrão deve ser equivalente a:

```bash
latexmk -pdf -interaction=nonstopmode -halt-on-error main.tex
```

Se o projeto exigir LuaLaTeX ou XeLaTeX, essa decisão deve ficar documentada no projeto, por exemplo em `latexmkrc` ou no README do projeto.

## Regra de integridade

Nunca inventar referências, autores, resultados, dados experimentais, medições, estatísticas, componentes utilizados, versões, datas ou conclusões. Informação ausente deve ser explicitamente registrada como pendência.
