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

Já é possível usar a linguagem para calcular a sequência de fibonacci usando 
loops:

	import 'lib/stdio';
	
	fn fibo(n) 
	{    
		a = long(1);
		b = long(1);
		i = uint(0);
		
		while (i < n) 
		{
		    la = a;
		    lb = b;
		    b = la + lb;
		    a = lb;
		    i += 1;
		};
		
		b;
	};
	
	print(fibo(90));

Já é possível calcular até o 91º elemento da sequência graças ao tipo long, a
ideia é implementar uma biblioteca para números de precisão abitrária para 
permitir que se calcule tantos elementos quanto a memória do sistema permitir.