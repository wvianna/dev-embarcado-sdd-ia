# Constituição do Projeto de Software

> Copie este arquivo para `.specs/project/constitution.md` e adapte os valores entre colchetes. A constituição contém princípios estáveis do projeto; requisitos específicos de uma feature pertencem a `features/<recurso>/spec.md`.

## Identidade do sistema

- Produto/sistema: [nome]
- Linguagem/framework: [linguagem, framework, runtime]
- Banco de dados: [tecnologia e versão]
- Infraestrutura/deploy: [provedor, orquestração, CI/CD]
- Ambientes de validação disponíveis: [local, CI, staging, produção]

## Princípios obrigatórios

### 1. Segurança

- Autenticação e autorização seguem [mecanismo definido pelo produto].
- Dados sensíveis (PII, credenciais, pagamento) devem ser [criptografados em trânsito/repouso, mascarados em logs].
- Entradas externas devem ser validadas e sanitizadas antes de qualquer uso em consultas, comandos ou renderização.
- Nenhuma alteração pode contornar controle de acesso, validação de entrada ou requisito de segurança sem decisão registrada.

### 2. Concorrência e consistência

- Estado compartilhado entre requisições, threads ou processos deve usar [política de sincronização] e ownership explícito.
- Operações críticas devem declarar idempotência, ordenação e comportamento em caso de execução concorrente.
- Transações devem definir isolamento, rollback e comportamento em caso de falha parcial.

### 3. Metas de qualidade e recursos

- Latência alvo: [valor]; throughput alvo: [valor]; disponibilidade alvo: [valor].
- Limites de custo, rate limit e quota: [valores].
- Persistência deve considerar integridade em falhas parciais e estratégia de migração/rollback.

### 4. Contratos e compatibilidade

- Endpoints, eventos e schemas de dados devem ser documentados antes da implementação.
- Mudanças incompatíveis (breaking changes) exigem versionamento e plano de migração para consumidores existentes.
- Formatos de mensagem devem definir schema, versionamento, valores obrigatórios/opcionais e comportamento para dados inválidos.

### 5. Qualidade e rastreabilidade

- Todo requisito funcional recebe um ID `FR-###`; todo requisito não funcional recebe `NFR-###`.
- Cada requisito alterado possui teste ou evidência; pendências devem registrar risco residual e responsável.
- O build deve ser reproduzível com versões registradas e lint/type-check tratados conforme [política].
- Código gerado por ferramenta não substitui revisão de contrato, segurança e comportamento.

### 6. Observabilidade e recuperação

- Logs, métricas e tracing devem respeitar [formato, nível, privacidade e custo de armazenamento].
- Falhas devem deixar evidência suficiente para diagnóstico (correlação de request, stack trace, contexto).
- A política de recuperação após falha é: [retry automático, fallback, degradar funcionalidade, exigir intervenção manual].

### 7. Processo de mudança

- A especificação é atualizada quando o comportamento aprovado muda; não se aceita corrigir apenas o código deixando o contrato obsoleto.
- Decisões que alteram risco, arquitetura, segurança, performance ou compatibilidade são registradas em `STATUS.md` ou no design da feature.
- Uma tarefa deve ser pequena o bastante para revisão e verificação isoladas.
- Não se adicionam abstrações, dependências ou camadas sem benefício verificável.

## Gates padrão

- [ ] Requisitos e critérios são observáveis e possuem IDs.
- [ ] Stack, versões e dependências foram confirmadas.
- [ ] Caminhos de erro, retry e estado de falha foram considerados.
- [ ] Segurança, concorrência e compatibilidade foram avaliadas quando aplicáveis.
- [ ] Testes foram executados no nível declarado: `LOCAL`, `CI`, `STAGING` ou `PRODUÇÃO`.
- [ ] O resultado e as limitações estão registrados.
