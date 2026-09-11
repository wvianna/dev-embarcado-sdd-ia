---
name: artigo-cientifico-latex
description: 'Produção, revisão e compilação de artigos científicos e tecnológicos em LaTeX (BibTeX/BibLaTeX), com rigor metodológico, referências verificáveis e elementos gráficos reproduzíveis. Use para criar artigo científico ou tecnológico, escrever paper em LaTeX, estruturar título/resumo/introdução/revisão/metodologia/resultados/discussão/conclusão, formatar para periódico ou congresso, revisar citações e referências, corrigir erros de compilação, validar o PDF. Nunca inventa dados, resultados ou referências; marca informações ausentes explicitamente.'
---

# Artigo Científico e Tecnológico em LaTeX

Skill para planejar, escrever, revisar e compilar artigos científicos e
tecnológicos em LaTeX, atuando como escritor, revisor técnico, editor e
especialista em LaTeX/BibTeX.

## Quando usar

- Criar um artigo a partir de tema, problema, hipótese, objetivos, metodologia
  e resultados fornecidos pelo usuário.
- Expandir, revisar ou corrigir um projeto LaTeX existente.
- Estruturar resumo, introdução, revisão da literatura, materiais e métodos,
  resultados, discussão e conclusão.
- Formatar para um periódico, congresso ou template institucional.
- Depurar erros/warnings de compilação e validar o PDF gerado.

## Modos de operação

1. **Criação** — artigo novo a partir das informações fornecidas.
2. **Expansão** — ampliar um artigo mantendo a estrutura existente.
3. **Revisão** — linguagem, estrutura, metodologia, coerência, referências.
4. **Revisão científica** — problema, hipótese, objetivos, método, resultados,
   discussão e conclusões.
5. **Revisão LaTeX** — sintaxe, pacotes, labels, citações, figuras, tabelas,
   compilação.
6. **Conversão** — converter um documento existente para LaTeX.
7. **Preparação para submissão** — adaptar a periódico/congresso/template.
8. **Compilação** — executar a compilação e corrigir problemas.

## Fluxo de trabalho

```
ENTRADAS → ANÁLISE DOS ARQUIVOS → TEMA → PROBLEMA → OBJETIVOS
→ REVISÃO DA LITERATURA → ESTRUTURA → REDAÇÃO → EQUAÇÕES/TABELAS/FIGURAS
→ REFERÊNCIAS → REVISÃO TÉCNICA/CIENTÍFICA → COMPILAÇÃO
→ ANÁLISE DE WARNINGS/ERRORS → CORREÇÕES → NOVA COMPILAÇÃO → VALIDAÇÃO
```

Antes de escrever, auditar o workspace em busca de: documentos, PDFs, `.tex`,
`.bib`, imagens, gráficos, dados experimentais, templates de periódicos e
instruções aos autores. Preservar a estrutura de projetos LaTeX já existentes.

## Estrutura padrão do artigo

Quando não houver template, usar:

```
Título
Autores + Afiliações
Resumo
Palavras-chave
1 Introdução
2 Fundamentação teórica / Revisão da literatura
3 Materiais e métodos
4 Resultados
5 Discussão
6 Conclusão
Agradecimentos
Referências
Apêndices/Anexos (quando necessário)
```

Artigos tecnológicos devem contemplar também: arquitetura do sistema,
desenvolvimento, implementação, validação, testes, desempenho, limitações e
reprodutibilidade.

## Princípio de integridade (obrigatório)

Nunca inventar: resultados, valores numéricos, referências, DOI, autores,
títulos, dados experimentais, citações, conclusões ou informações metodológicas
não fornecidas. Para informação ausente, usar marcadores explícitos, ex.:

- `[DADO A INSERIR]`
- `[REFERÊNCIA NECESSÁRIA]` / `[REFERÊNCIA A VALIDAR]`
- `[RESULTADO EXPERIMENTAL NÃO INFORMADO]`
- `[FIGURA A INSERIR]`

Nunca transformar hipótese em resultado. Não mascarar pendências para simular um
artigo concluído.

## Diretrizes de escrita por seção

### Título
Objetivo, técnico, sem exageros nem afirmações não comprovadas. Gerar 3–5
alternativas e escolher a mais adequada.

### Resumo
Condensar contexto, problema, objetivo, metodologia, principais resultados e
conclusão. Sem citações nem informação ausente do texto. Palavras-chave devem
refletir os conceitos centrais.

### Introdução
Progressão: contexto → problema → lacuna → justificativa → objetivo →
contribuição → organização do artigo. Responder: qual o problema, por que
importa, o que já foi feito, qual a lacuna, o que se propõe, qual a contribuição.

### Revisão da literatura
Crítica e comparativa, não "Autor A fez X; Autor B fez Y". Estruturar:
problema → estado da arte → limitações das abordagens → lacuna → contribuição.
Priorizar fontes confiáveis (IEEE, Springer, Elsevier, ACM, ScienceDirect,
Scopus, SciELO, PubMed, normas, fontes oficiais).

### Metodologia
Permitir reprodução: materiais, equipamentos, sensores, software, hardware,
parâmetros, condições, procedimentos, algoritmos, métricas e critérios de
validação. Em artigos tecnológicos: arquitetura, componentes, protocolos,
interfaces, comunicação, firmware, infraestrutura e limitações.

### Resultados e discussão
Separar resultado observado → interpretação → comparação com a literatura →
implicação. A discussão responde: o resultado era esperado? Como se compara?
Quais fatores explicam? Limitações? Contribuição? Implicações?

### Conclusão
Retomar o objetivo, indicar se foi atingido, sintetizar descobertas e
contribuições, apontar limitações e trabalhos futuros. Não introduzir resultados
inéditos.

## Equações, figuras, tabelas e gráficos

- **Equações**: numeração quando necessário, `\label{}`, referência no texto
  (`A Equação~\ref{eq:x}...`) e definição de todas as variáveis.
- **Figuras**: em `figures/`, com `\caption{}`, `\label{}`, referenciadas no
  texto, resolução adequada e fonte quando necessário.
- **Tabelas**: preferir `booktabs`; legenda, `label` e referência no texto.
- **Gráficos**: reproduzíveis com `pgfplots`; preservar os dados usados.
- **Diagramas**: TikZ/PGFPlots (vetorial); Mermaid apenas para documentação.

## Referências e citações

- Manter referências em `references.bib` com identificadores consistentes
  (ex.: `@article{silva2025monitoramento,...}`).
- Preferir `biblatex` (ou o sistema do template); estilo ABNT quando for o caso.
- Verificar autores, ano, título, periódico, volume, número, páginas, DOI, URL.
- Garantir correspondência `Afirmação → Citação → Referência`; nenhuma
  referência sem uso, salvo pedido explícito.
- Toda afirmação dependente de literatura deve ter suporte bibliográfico.

## Estrutura do projeto LaTeX

```
article/
├── main.tex
├── references.bib
├── README.md
├── .gitignore
├── sections/ (introduction, literature, methodology, results, discussion, conclusion)
├── figures/  tables/  data/  scripts/
└── build/ (article.pdf)
```

Use o template em [`templates/main.tex`](./templates/main.tex). O `main.tex`
centraliza o preâmbulo, as entradas das seções e as referências.

## Compilação

- Com `latexmk` (preferencial): `latexmk -pdf -interaction=nonstopmode main.tex`
- Com Biber: `latexmk -pdf -use-biber -interaction=nonstopmode main.tex`
- Manual: `pdflatex` → `biber`/`bibtex` → `pdflatex` → `pdflatex`.

## Tratamento de erros e validação do PDF

1. Verificar erros, warnings, referências/citações indefinidas, figuras
   ausentes, tabelas/equações quebradas e referências cruzadas.
2. Inspecionar o PDF: título, autores, resumo, seções, numeração, figuras,
   tabelas, equações, referências, hyperlinks, paginação, elementos fora da
   margem e texto cortado.

## Checklist final

- Conteúdo: título, resumo, palavras-chave, problema, objetivo, fundamentação,
  metodologia, resultados, discussão e conclusão coerentes.
- Referências: todas as citações resolvidas, referências citadas e verificadas.
- Elementos: figuras/tabelas com legenda, `label` e referência no texto.
- LaTeX: compilação sem erros, citações/referências cruzadas resolvidas, PDF
  gerado e inspecionado.

## Recursos

- [`templates/main.tex`](./templates/main.tex) — preâmbulo e estrutura de exemplo.
- [`templates/references.bib`](./templates/references.bib) — exemplos de entradas.
- [`templates/README.md`](./templates/README.md) — README do projeto do artigo.
- [`templates/gitignore`](./templates/gitignore) — exclusões de temporários LaTeX.
