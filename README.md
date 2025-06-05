
# Mini-shell

Este projeto implementa um shell simples em C, capaz de executar comandos internos e externos, lidar com pipes, redirecionamento de saída e processamento de arquivos batch. Ele também suporta a execução concorrente de comandos usando threads.


## Requisitos do sistema
Para compilar e rodar este shell, você precisará dos seguintes requisitos em seu sistema operacional Linux (idealmente, dentro de uma Máquina Virtual):

**Sistema Operacional**: Linux (testado em distribuições baseadas em Debian/Ubuntu, mas deve funcionar em qualquer sistema POSIX compatível).

**Compilador**: GCC (GNU Compiler Collection).

**Ferramentas**: VirtualBox (ou outra plataforma de virtualização).
## Clone o repositório
Clone o repositório do github na pasta que deseja com o comando:

```git clone https://github.com/murilosf94/minishell.git```
## Configuração da Máquina Virtual (VM) e Pasta Compartilhada
Para desenvolver e usar o shell de forma organizada, o uso de uma Máquina Virtual com uma pasta compartilhada é altamente recomendado. Este guia assume que você está usando o VirtualBox e já está logado no seu servidor.

**1. Configurar a Pasta Compartilhada no VirtualBox:**

Esta etapa é feita no VirtualBox VM (o programa do VirtualBox na sua máquina real).

Ao ligar a sua máquina virtual, nas opções que ficam na parte de cima, vá para "*Dispositivos*", depois "*Pastas Compartilhadas*", clique no ícone de "Adicionar pasta compartilhada" (uma pasta com um sinal de + verde).

Em "Caminho da Pasta", clique em "Outro" e navegue até a pasta em seu sistema hospedeiro (sua máquina real) onde o projeto do *Mini-shell* está.

Marque as opções:

``Somente leitura``: NÃO marque esta opção, pois você precisará escrever (compilar) dentro da pasta.

``Montagem automática``: Marque esta opção. Isso garantirá que a pasta seja montada automaticamente quando a VM iniciar.

``Tornar Permanente``: Marque esta opção para que a configuração seja salva.

Clique em "OK" e novamente em "OK" para fechar as configurações da VM.

## Conecte a máquina hospedeira com a máquina virtual
Agora, com a máquina virtual ligada e logada, entre no cmd da sua máquina hospedeira e digite:

```ssh <usuário>@127.0.0.1 -p2222```

e digite sua senha.



## Executando o código
Após digitar sua senha e conectar a máquina hospedeira com a máquina virtual, digite:

```sudo mount -t vboxsf minishell minishell/``` (assumindo que o nome da pasta em que o projeto está chama minishell)

Logo após isso, você estará com a pasta montada, então navegue (utilizando o comando *cd*) até a pasta que se encontra o mini-shell, então digite:

```gcc -g -o shell shell.c```

para compilar o código, então execute com:

```./shell```

pronto, o mini-shell estará rodando!!!
## Funcionalidades

- cd
- ls
- pwd
- path
- pipe
- cat
- redirecionamento (>)
- exit
- mais as variáveis padrões do sistema (wc, grep, ps...)


## Explicando cada funcionalidade

### cd (change directory)
---
Comando utilizado para mudar o diretório e navegar pelas pastas.
Exemplo:

```cd /caminho/da/pasta```

### ls (list)
---
Comando utilizado para listar o conteúdo de um diretório.
Exemplo:

```ls```

```ls -l``` (lista longa)

```ls -a``` (todos, incluindo ocultos)

```ls -la```
```ls -al```

### pwd
---
Comando utilizado para exibir o caminho completo do diretório atual em que você está no terminal.

```pwd```

### path
---
Comando utilizado para adicionar um caminho a uma lista de diretórios onde o sistema operacional busca por executáveis quando você digita um comando na linha de comando. 
Exemplo para criar um executável e colocar no path:

- Antes de executar o mini-shell (ainda no seu servidor)
- Navegue até a pasta que quer adicionar o executável (vamos imaginar que seja */tmp*)
- Crie o diretório. Exemplo: ```mkdir /tmp/meusbin```

**Dentro de */tmp/meusbin* crie o executável, vamos usar de exemplo um executável para soma.**

```
    cat << 'EOF' > soma.sh
    #!/bin/bash
    # Este script soma dois números fornecidos como argumentos.

    if [ "$#" -ne 2 ]; then
        echo "Uso: $0 <numero1> <numero2>"
        exit 1
    fi

    numero1=$1
    numero2=$2

    soma=$(($numero1 + $numero2))

    echo "A soma de $numero1 e $numero2 é: $soma"
    EOF
```
**Dê permissão de execução:**

Após criar o arquivo, você precisa torná-lo executável:

```
chmod +x soma.sh
```
- **Agora execute o shell normalmente**
Agora digite:

```path /tmp/meusbin```
(para adicionar o caminho */tmp/meusbin*)

Agora esse caminho faz parte da busca por executáveis, então quando você digitar:

```soma.sh 7 3```

o shell imprimirá o resultado da soma dos argumentos mandados, nesse caso 7 e 3.

Quando você digitar

```path```(sem argumentos)

O programa limpará todos os *paths* e voltará a utilizar apenas o do sistema.

### cat
---
Comando utilizado para exibir o conteúdo de arquivos de texto, bem como para criar e concatenar arquivos
Exemplo:

```cat arquivo.txt```

### pipe
---
Comando utilizado para que a saída de um comando seja usada como entrada para outro comando, criando uma sequência de processamento de dados.
Exemplo:

```ls | grep shell```

### redirecionamento (>)
---
Comando utilizado para redirecionar a saída de um comando para um arquivo ou para um fluxo de entrada de outro comando.
Exemplo:

 ```ls -l > arquivo.txt```
 (a saída de ls vai estar em *arquivo.txt*)

### exit
---
Comando utilizado para sair do mini-shell.

```exit```
## Conclusão

O desenvolvimento deste **minishell** foi uma jornada de aprendizado desafiadora e recompensadora, aprofundando nosso entendimento sobre os princípios de um shell Unix básico e a programação de sistemas.

Agradecemos o seu interesse neste projeto. Esperamos que ele seja útil como ferramenta ou como fonte de estudo. Estamos sempre abertos a **feedback** e **sugestões** para aprimoramentos. Se você encontrou algum problema, tem ideias para novos recursos, ou gostaria de contribuir com o código, sinta-se à vontade para **abrir uma *issue*** ou enviar um ***pull request***. Sua colaboração é muito valorizada!


## Autores
- [Murilo de Souza Freitas](https://github.com/murilosf94)
- [Gabriel Geraldo Mazolli](https://github.com/Gagema)
- [João Pedro Kafer Bachiega](https://github.com/joaokafer)
- [João Vitor de Athayde Abram](https://github.com/JoaoAbram)
- [Raphael Borges Fagundes](https://github.com/RaphaelBFagundes)
- [Matheus Henrique de Oliveira César](https://github.com/mikezin021)
- [Matheus de Oliveira Batista](https://github.com/Kraken2k)
- [Renato Hildebrand Pissinatti](https://github.com/RenatoHPissinatti)
