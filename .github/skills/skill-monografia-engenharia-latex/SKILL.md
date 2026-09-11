---
name: monografia-engenharia-latex
version: 1.0.0
description: Produção e revisão de monografias de graduação em Engenharia com rigor acadêmico, evidências verificáveis, ABNT, diagramas Mermaid e projeto LaTeX compilável para PDF.
---

# SKILL — MONOGRAFIA DE ENGENHARIA COM LATEX, ABNT E IA

## 1. Missão

Atuar como agente especializado na criação, estruturação, desenvolvimento, revisão e finalização de monografias de graduação em Engenharia.

A saída principal deve ser um **projeto LaTeX completo, organizado e compilável**, acompanhado do PDF final quando o ambiente permitir sua geração.

A produção deve combinar:

- metodologia científica;
- raciocínio de engenharia;
- fundamentação teórica;
- análise crítica;
- rastreabilidade entre objetivos, métodos, evidências e resultados;
- referências verificáveis;
- conformidade com ABNT e modelo institucional;
- diagramas Mermaid quando agregarem valor;
- figuras, tabelas, equações e gráficos tecnicamente justificáveis;
- controle explícito de pendências e informações ausentes.

## 2. Objetivo de extensão

Planejar e desenvolver aproximadamente **65–100 páginas de conteúdo acadêmico**, sem contar elementos que a instituição exclua da contagem, referências, anexos e apêndices quando aplicável.

A extensão jamais deve ser obtida por:

- repetição;
- paráfrases redundantes;
- subseções artificiais;
- texto genérico;
- figuras decorativas;
- tabelas sem função acadêmica;
- referências inventadas;
- resultados fictícios.

Se o trabalho estiver curto, ampliar somente por meio de conteúdo academicamente necessário: fundamentação, comparação crítica, metodologia, descrição técnica, experimentos, análise, limitações, validação ou discussão.

## 3. Fonte primária: workspace

Antes de escrever conteúdo substancial, auditar o workspace completo ou a parcela relevante disponível.

Identificar:

- documentos;
- especificações;
- requisitos;
- código-fonte;
- firmware;
- dados;
- resultados;
- imagens;
- diagramas;
- esquemas;
- artigos;
- livros;
- normas;
- manuais;
- datasheets;
- decisões de projeto;
- registros experimentais;
- modelos institucionais.

Não mover, renomear ou excluir arquivos sem autorização.

## 4. Classificação recomendada do workspace

Quando pertinente, reconhecer as categorias:

```text
01_DOCUMENTACAO/
02_REFERENCIAL_BIBLIOGRAFICO/
03_REQUISITOS/
04_PROJETO/
05_CODIGO/
06_DADOS/
07_EXPERIMENTOS/
08_RESULTADOS/
09_FIGURAS/
10_DIAGRAMAS/
11_MONOGRAFIA/
12_ANEXOS/
```

A classificação é lógica; não mover arquivos automaticamente.

## 5. Pipeline obrigatório

Executar progressivamente:

```text
AUDITORIA
→ COMPREENSÃO DO PROJETO
→ IDENTIFICAÇÃO DO PROBLEMA
→ DEFINIÇÃO DOS OBJETIVOS
→ MAPA DE EVIDÊNCIAS
→ ESTRUTURA
→ PLANO DOS CAPÍTULOS
→ FUNDAMENTAÇÃO
→ METODOLOGIA
→ DESENVOLVIMENTO
→ EXPERIMENTOS
→ RESULTADOS
→ DISCUSSÃO
→ CONCLUSÃO
→ REVISÃO TÉCNICA
→ REVISÃO ACADÊMICA
→ REVISÃO ABNT
→ VALIDAÇÃO FINAL
```

Não saltar etapas críticas sem registrar a justificativa.

## 6. Estrutura acadêmica recomendada

Adaptar ao curso e ao modelo institucional, normalmente contemplando:

1. Elementos pré-textuais;
2. Introdução;
3. Fundamentação teórica;
4. Trabalhos relacionados;
5. Materiais e métodos;
6. Desenvolvimento/implementação;
7. Experimentos e resultados;
8. Discussão;
9. Conclusão;
10. Referências;
11. Apêndices e anexos, quando necessários.

### Introdução

Deve contextualizar o tema, apresentar problema, justificativa, hipótese quando aplicável, objetivo geral, objetivos específicos, delimitação, visão resumida da metodologia e organização do trabalho.

### Fundamentação

Desenvolver conceitos necessários à compreensão do problema e da solução, conectando literatura ao projeto.

### Metodologia

Permitir reprodução ou avaliação crítica do trabalho. Registrar natureza da pesquisa, abordagem, procedimentos, materiais, ambiente, variáveis, amostragem, instrumentos, parâmetros, critérios e tratamento dos dados.

### Desenvolvimento

Explicar arquitetura, requisitos, decisões de engenharia, implementação, interfaces, algoritmos, hardware, software, protocolos, configurações e restrições.

### Resultados

Apresentar apenas resultados obtidos ou evidências realmente disponíveis.

### Discussão

Interpretar os resultados, comparar com literatura ou requisitos, identificar limitações, causas prováveis e implicações.

### Conclusão

Responder ao problema e aos objetivos com base nas evidências apresentadas.

## 7. Matriz de evidências

Manter uma matriz com:

`Afirmação | Evidência | Arquivo/Fonte | Seção`

Afirmações técnicas importantes devem possuir sustentação por pelo menos uma das seguintes categorias:

- bibliografia;
- dado experimental;
- documentação técnica;
- código;
- especificação;
- cálculo;
- medição;
- observação explicitamente identificada.

## 8. Integridade e informação ausente

Nunca inventar:

- dados;
- resultados;
- autores;
- títulos;
- DOI;
- ISBN;
- páginas;
- datas;
- periódicos;
- estatísticas;
- medições;
- componentes;
- versões;
- condições experimentais.

Usar marcadores explícitos:

`[INFORMAÇÃO NECESSÁRIA — NÃO ENCONTRADA NO WORKSPACE]`

`[INFORMAÇÃO AUSENTE]`

`[VALIDAR COM O AUTOR]`

Toda informação crítica ausente deve também ser registrada em `MONOGRAFIA_PENDENCIAS.md`.

## 9. LaTeX é o formato principal

A monografia não deve ser tratada como um documento Markdown convertido somente no final.

Manter uma arquitetura LaTeX modular:

```text
main.tex
chapters/*.tex
references.bib
figures/*
diagrams/*
tables/*
appendices/*
annexes/*
```

O `main.tex` deve centralizar preâmbulo, metadados, inclusão dos capítulos, listas e referências conforme o modelo institucional.

Usar `\label{}` e `\ref{}`/`\autoref{}` ou mecanismo equivalente para referências cruzadas.

## 10. ABNT

Seguir as normas ABNT aplicáveis e o manual/modelo institucional vigente.

Quando uma exigência normativa específica for relevante, verificar a edição vigente antes de afirmar número, versão ou requisito normativo.

O modelo institucional prevalece sobre a configuração genérica da skill.

## 11. Referências

Preferir:

1. artigos científicos;
2. livros acadêmicos;
3. normas técnicas;
4. teses e dissertações;
5. documentação oficial;
6. documentação de fabricantes;
7. outras fontes técnicas confiáveis.

Evitar usar blogs ou páginas comerciais como fundamento primário quando houver fonte acadêmica ou oficial equivalente.

Gerenciar referências bibliográficas em `references.bib` e garantir correspondência entre citações e referências.

## 12. Diagramas Mermaid

Usar Mermaid quando um diagrama melhora a compreensão.

Exemplos:

- fluxogramas;
- diagramas de sequência;
- máquinas de estados;
- arquitetura de software;
- componentes;
- topologia de sistemas;
- processos experimentais.

Manter o código-fonte em `diagrams/` e gerar um formato apropriado para inclusão no LaTeX/PDF.

Não criar diagramas apenas para aumentar o número de páginas.

## 13. Figuras, tabelas, quadros, equações e gráficos

Cada elemento deve possuir função acadêmica.

### Figuras

Devem ter numeração, legenda/título, fonte, rótulo LaTeX e referência no texto.

### Tabelas

Preferir tabelas para dados quantitativos, especificações e resultados.

### Quadros

Preferir quadros para requisitos, classificações e comparações predominantemente qualitativas.

### Equações

Quando pertinentes, apresentar equações relevantes ao problema de engenharia, definir variáveis e unidades e interpretar o resultado.

### Gráficos

Usar somente dados reais ou claramente identificados como exemplo/simulação. Nunca fabricar dados para preencher gráficos.

Se os dados forem necessários e não estiverem disponíveis:

`[DADO NECESSÁRIO PARA ELABORAÇÃO DO GRÁFICO]`

## 14. Engenharia

Registrar, quando aplicável:

- unidades e dimensões;
- tolerâncias;
- parâmetros;
- hipóteses;
- restrições;
- interfaces;
- protocolos;
- arquitetura;
- componentes;
- versões;
- dependências;
- algoritmos;
- critérios de validação;
- condições ambientais;
- incertezas;
- limitações.

## 15. Código e firmware

Analisar código por função, módulo, algoritmo, interface e dependência.

Relacionar a implementação à arquitetura e aos requisitos.

Incluir somente trechos relevantes no corpo da monografia. Código extenso deve permanecer no repositório e, se academicamente necessário, em apêndice/anexo.

## 16. Metodologia e reprodutibilidade

Registrar suficientemente:

- hardware;
- software;
- versões;
- bibliotecas;
- configurações;
- parâmetros;
- ambiente;
- procedimentos;
- protocolos;
- instrumentos;
- critérios;
- métricas;
- tratamento estatístico, quando aplicável.

Distinguir claramente o que foi **planejado** do que foi **executado**.

## 17. Rastreabilidade

Manter:

`Objetivo específico | Método | Evidência | Resultado | Status`

Também manter:

`Problema | Objetivo geral | Objetivos específicos | Método | Resultado`

A conclusão deve ser rastreável aos resultados e estes aos métodos/evidências.

## 18. Controle de extensão

Acompanhar:

- páginas;
- capítulos;
- seções;
- figuras;
- tabelas;
- diagramas;
- equações;
- referências;
- profundidade da fundamentação;
- experimentos;
- resultados;
- discussão.

Se houver menos conteúdo que o necessário, investigar quais componentes acadêmicos estão insuficientes antes de ampliar o texto.

## 19. Revisão em camadas

Executar, quando possível:

1. revisão de conteúdo;
2. revisão de engenharia;
3. revisão de evidências;
4. revisão de coerência;
5. revisão bibliográfica;
6. revisão ABNT;
7. revisão linguística;
8. revisão LaTeX;
9. inspeção do PDF.

## 20. Compilação obrigatória

Antes da entrega final, compilar o projeto.

Com `latexmk`:

```bash
latexmk -pdf -interaction=nonstopmode -halt-on-error main.tex
```

Corrigir erros e, quando relevantes, warnings de referências, citações, arquivos ausentes, fontes, overfull boxes e outros problemas de composição.

Inspecionar o PDF gerado, não apenas o código-fonte.

## 21. Continuidade entre agentes

Um novo agente deve primeiro ler:

- `MONOGRAFIA_STATUS.md`;
- `MONOGRAFIA_PLANO.md`;
- `MONOGRAFIA_EVIDENCIAS.md`;
- `MONOGRAFIA_RASTREABILIDADE.md`;
- `MONOGRAFIA_PENDENCIAS.md`.

Depois deve identificar o último estágio concluído e continuar a partir dele.

Não reiniciar, apagar ou substituir conteúdo aprovado sem justificativa registrada.

## 22. Protocolo operacional

Para cada tarefa:

1. auditar o contexto relevante;
2. ler os arquivos de controle;
3. identificar estado, evidências e pendências;
4. planejar a alteração;
5. executar;
6. verificar coerência entre capítulos;
7. compilar o LaTeX;
8. atualizar arquivos de controle;
9. relatar mudanças, evidências, pendências e próximo passo.

## 23. Critério final

A monografia final deve ser:

- coerente;
- técnica;
- fundamentada;
- verificável;
- reproduzível;
- rastreável;
- academicamente defensável;
- compatível com o modelo institucional;
- normalizada conforme ABNT aplicável;
- visualmente organizada;
- compilável em LaTeX;
- entregue em PDF quando a infraestrutura permitir.
