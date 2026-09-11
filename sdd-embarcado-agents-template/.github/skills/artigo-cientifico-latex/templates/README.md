# <Título do artigo>

## Autores
- <Autor A>
- <Autor B>

## Objetivo
<Resumo do objetivo do artigo.>

## Estrutura do projeto
```
main.tex            # preâmbulo e entrada das seções
references.bib      # referências bibliográficas
sections/           # introduction, literature, methodology, results, discussion, conclusion
figures/            # figuras
tables/             # tabelas
data/               # dados experimentais
scripts/            # scripts de geração de gráficos
build/              # PDF gerado
```

## Requisitos e dependências
- Distribuição TeX (TeX Live/MiKTeX) com `latexmk` (ou `pdflatex` + `biber`/`bibtex`).
- Pacotes: `amsmath`, `amssymb`, `graphicx`, `booktabs`, `float`, `pgfplots`,
  `hyperref`, `cleveref` e `biblatex` (opcional, para ABNT).

## Como compilar
```bash
latexmk -pdf -interaction=nonstopmode main.tex        # com bibtex
latexmk -pdf -use-biber -interaction=nonstopmode main.tex  # com biblatex/biber
```

## Licença
<Definir, quando aplicável.>
