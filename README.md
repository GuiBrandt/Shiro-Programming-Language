「白」プログラミング言語
---

Yo!

Se você chegou aqui provavelmente sabe do que se trata o repositório, mas pra não ficar faltando uma explicação:
O Shiro Programming Language (ou shiro pra encurtar) é um compilador/interpretador/qualquer coisa pra uma linguagem bem simples, com sintaxe semelhante a C.

O programa é todo feito em C e o único objetivo do projeto é entender o funcionamento de um interpretador/compilador/qualquer coisa.

O nome é aleatório mesmo. Deal with it.


Progresso
---

Já é possível usar a linguagem para calcular a sequência de fibonacci de forma recursiva:

	import('lib/stdio');

	fn fibo(n)
	{
	    if ((n == 0) | (n == 1))
	    {
	        1;
	    }
	    else
	    {
	        fibo(n - 1) + fibo(n - 2);
	    };
	};
	
	print(fibo(20));