「白」プログラミング言語
---

Yo!

Se você chegou aqui provavelmente sabe do que se trata o repositório, mas pra não ficar faltando uma explicação:
O Shiro Programming Language (ou shiro pra encurtar) é um compilador/interpretador/qualquer coisa pra uma linguagem bem simples, com sintaxe semelhante a C.

O programa é todo feito em C e o único objetivo do projeto é entender o funcionamento de um interpretador/compilador/qualquer coisa.

O nome é aleatório mesmo. Deal with it.


Progresso
---

Já é possível usar a linguagem para calcular a sequência de fibonacci usando loops:

	import 'lib/stdio';
	
	fn fibo(n) 
	{    
		var a = 1;
		var b = 1;
		var i = 0;
		
		while (i < n) 
		{
		    var la = a;
		    var lb = b;
		    var b = la + lb;
		    var a = lb;
		    var i = i + 1;
		};
		
		b;
	};
	
	print(fibo(44));

Por enquanto não tem como calcular do 45º elemento da sequência pra frente por conta do limite de 4 bytes dos números inteiros.