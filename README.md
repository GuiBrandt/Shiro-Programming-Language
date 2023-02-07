Shiro
===

Este repositório contém o código do interpretador da linguagem "Shiro", uma linguagem C-like
desenvolvida em 2016 como estudo. O objetivo deste projeto era o de tentar entender o funcionamento
de um interpretador/compilador de uma linguagem de programação.

O projeto inclui:
- [Uma implementação de Lexer](./src/shiro/lexer.c)
- [Uma implementação de Parser de descida recursiva](./src/shiro/parser.c)
- [Uma implementação de interpretador/máquina virtual](./src/shiro/eval.c)
- [Bibliotecas padrão para a linguagem](./src/shiro/libs)

Infelizmente, devido a limitações de conhecimento do autor à época, nenhum tipo de otimização foi
implementada como parte do pipeline do compilador.

Exemplo
---

Um exemplo de programa para para calcular a sequência de fibonacci indeterminadamente usando loops
e números de precisão arbitrária na linguagem:
```rust
import "bignum";

fn fibo(n) {
	a = bignum(1);
	b = bignum(1);

	i = uint(0);

	while (i < n) {
		lb = b;
		b = bignum_add(a, b);
		a = lb;

		i += 1;
	};

	free_bignum(a);

	return b;
};

import "stdio";

print(bignum_to_string(fibo(9000)));
```
