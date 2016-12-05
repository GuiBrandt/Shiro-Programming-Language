「白」プログラミング言語
---

Yo!

Se você chegou aqui provavelmente sabe do que se trata o repositório, mas pra 
não ficar faltando uma explicação:
O Shiro Programming Language (ou shiro pra encurtar) é um 
compilador/interpretador/qualquer coisa pra uma linguagem bem simples, com 
sintaxe semelhante a C.

O programa é todo feito em C e o único objetivo do projeto é entender o 
funcionamento de um interpretador/compilador/qualquer coisa.

O nome é aleatório mesmo. Deal with it.


Progresso
---

Já é possível usar a linguagem para calcular a sequência de fibonacci 
indeterminadamente usando loops e números de precisão arbitrária:

	import "lib/bignum";

	fn fibo(n) 
	{
		a = bignum(1);
		b = bignum(1);
		
		i = uint(0);
		
		while (i < n) 
		{
			lb = b;
			b = bignum_add(a, b);
			a = lb;
			
			i += 1;
		};
		
		free_bignum(a);
		
		b;
	};

	import "lib/stdio";

	print(bignum_to_string(fibo(9000)));