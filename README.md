#「白」プログラミング言語
---

Yo!

Se você chegou aqui provavelmente sabe do que se trata o repositório, mas pra não ficar faltando uma explicação:
O Shiro Programming Language (ou shiro pra encurtar) é um compilador/interpretador/qualquer coisa pra uma linguagem bem simples, com sintaxe semelhante a C.

O programa é todo feito em C e o único objetivo do projeto é entender o funcionamento de um interpretador/compilador/qualquer coisa.

O nome é aleatório mesmo. Deal with it.


Progresso
---

No momento o shiro compila sentenças if/else e chama funções nativas (escritas em C e passadas para o ambiente de execução da linguagem por ponteiro).

Os tempos de execução e compilação estão em aproximadamente 0.01ms e 0.1ms respectivamente para o seguinte código:

    if (1) {
		print('oe');
	} else {
		print('tiao');
	};
	
	print('asd');
	print('bsd');
