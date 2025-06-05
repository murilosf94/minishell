Mini-Shell CEste projeto implementa um shell simples em C, capaz de executar comandos internos e externos, lidar com pipes, redirecionamento de saída e processamento de arquivos batch. Ele também suporta a execução concorrente de comandos usando threads.Requisitos do SistemaPara compilar e rodar este shell, você precisará dos seguintes requisitos em seu sistema operacional Linux (idealmente, dentro de uma Máquina Virtual):Sistema Operacional: Linux (testado em distribuições baseadas em Debian/Ubuntu, mas deve funcionar em qualquer sistema POSIX compatível).Compilador: GCC (GNU Compiler Collection).Ferramentas: git (para clonar o repositório), ssh (opcional, para acesso remoto à VM), VirtualBox (ou outra plataforma de virtualização).Configuração da Máquina Virtual (VM) e Pasta CompartilhadaPara desenvolver e testar o shell de forma organizada, o uso de uma Máquina Virtual com uma pasta compartilhada é altamente recomendado. Este guia assume que você está usando o VirtualBox.1. Configurar a Pasta Compartilhada no VirtualBox (Na Máquina Hospedeira)Esta etapa é feita no VirtualBox Manager (o programa do VirtualBox na sua máquina real).Desligue sua VM: Certifique-se de que a máquina virtual esteja completamente desligada.Acesse as Configurações da VM: No VirtualBox Manager, selecione sua VM e clique em "Configurações" (ou use Ctrl + S).Vá para "Pastas Compartilhadas": No menu lateral esquerdo, clique em "Pastas Compartilhadas".Adicionar Nova Pasta Compartilhada:Clique no ícone de "Adicionar pasta compartilhada" (uma pasta com um sinal de + verde).Em "Caminho da Pasta", clique em "Outro" e navegue até a pasta em seu sistema hospedeiro (sua máquina real) onde o projeto do mini-shell será clonado. Por exemplo, você pode criar uma pasta chamada minishell_dev em seus documentos.Em "Nome da Pasta", digite um nome curto e sem espaços, por exemplo: minishell_proj.Marque as opções:Somente leitura: NÃO marque esta opção, pois você precisará escrever (compilar) dentro da pasta.Montagem automática: Marque esta opção. Isso garantirá que a pasta seja montada automaticamente quando a VM iniciar.Tornar Permanente: Marque esta opção para que a configuração seja salva.Clique em "OK" e novamente em "OK" para fechar as configurações da VM.2. Instalar Guest Additions na VM (Dentro da VM)Os VirtualBox Guest Additions são cruciais para o funcionamento adequado das pastas compartilhadas e outros recursos de integração.Inicie sua VM.No menu do VirtualBox (da janela da VM): Clique em Dispositivos > Inserir Imagem de CD de Convidado....Dentro da VM, abra um terminal e siga estes passos:Atualize os pacotes do sistema:sudo apt update
Instale as dependências necessárias para compilar os Guest Additions:sudo apt install build-essential linux-headers-$(uname -r) dkms -y
Navegue até o diretório onde a imagem de CD dos Guest Additions foi montada (geralmente em /media/seu_usuario/VBox_GAs_...). Você pode verificar com ls /media/$(whoami)/.cd /media/$(whoami)/VBox_GAs_*/ # Substitua VBox_GAs_* pelo nome exato da pasta
Execute o script de instalação:sudo ./VBoxLinuxAdditions.run
Reinicie a VM após a instalação bem-sucedida:sudo reboot
3. Acessar a Pasta Compartilhada dentro da VM (Dentro da VM)Após reiniciar a VM, a pasta compartilhada será montada automaticamente. No Linux, as pastas compartilhadas do VirtualBox são montadas em /media/sf_<nome_da_pasta_compartilhada>.Verifique se a pasta está montada:ls /media/sf_minishell_proj/
Você deve ver o conteúdo da pasta que você compartilhou do seu sistema hospedeiro.Permissões (Opcional, se houver problemas de acesso): Se você tiver problemas de permissão (não consegue criar/modificar arquivos), adicione seu usuário ao grupo vboxsf e reinicie a VM:sudo usermod -aG vboxsf $(whoami)
sudo reboot
4. Acessar a VM via SSH (Opcional, mas Recomendado)Para uma experiência de desenvolvimento mais fluida, você pode configurar o acesso SSH à sua VM a partir da máquina hospedeira.Instale o servidor SSH na VM:sudo apt update
sudo apt install openssh-server -y
Obtenha o IP da sua VM: Dentro da VM, execute:ip a
# Procure o endereço IP na interface de rede (geralmente eth0 ou enp0s3)
No seu sistema hospedeiro, conecte via SSH:ssh seu_usuario_vm@ip_da_sua_vm
# Exemplo: ssh user@192.168.1.105
Digite a senha do seu usuário na VM quando solicitado. A partir daqui, você pode operar a VM a partir do terminal da sua máquina hospedeira.Preparando o Projeto1. Clonar o Repositório (Na Máquina Hospedeira, na Pasta Compartilhada)É uma boa prática clonar o repositório no seu sistema hospedeiro e acessá-lo através da pasta compartilhada na VM.Abra um terminal no seu sistema hospedeiro.Navegue até a pasta que você configurou como compartilhada no VirtualBox:cd /caminho/para/sua/pasta/compartilhada/minishell_dev/
# Exemplo: cd ~/Documentos/minishell_dev/
Clone o repositório do seu mini-shell:git clone https://github.com/seu-usuario/seu-minishell.git .
# O ponto final '.' no final do comando clona o conteúdo do repositório diretamente na pasta atual.
# Certifique-se de substituir o URL pelo link real do seu repositório Git.
Agora, o código-fonte do seu mini-shell estará acessível dentro da VM na pasta /media/sf_minishell_proj/ (ou o nome que você deu à pasta compartilhada).Compilando o Mini-ShellDentro do terminal da sua VM (seja acessando-o diretamente na VM ou via SSH):Navegue até a pasta compartilhada onde o projeto está localizado:cd /media/sf_minishell_proj/
# Ou o caminho para onde você clonou o projeto.
Compile o programa usando o GCC:gcc -g -o shell shell.c -pthread
-g: Inclui informações de depuração no executável. Isso é útil se você precisar depurar o código com ferramentas como o GDB.-o shell: Define o nome do arquivo de saída (o executável) como shell.shell.c: O arquivo fonte principal do seu mini-shell.-pthread: Essencial para vincular a biblioteca de threads POSIX (pthread), pois seu shell utiliza threads para a execução concorrente de comandos.Executando o Mini-ShellApós a compilação bem-sucedida, você pode executar seu mini-shell:./shell
Você verá o prompt do seu shell: /> Funcionalidades e Exemplos de UsoSeu mini-shell implementa diversas funcionalidades comuns a um shell Linux:1. Comandos Internos (Built-in)Estes comandos são processados e executados diretamente pelo seu shell, sem a necessidade de iniciar um novo processo externo.exit - Sair do ShellEncerra a execução do mini-shell./> exit
cd <diretório> - Mudar de DiretórioAltera o diretório de trabalho atual do shell./> pwd
/home/seu_usuario_vm/minishell_proj
/> cd ..
/> pwd
/home/seu_usuario_vm/
/> cd minishell_proj
/> pwd
/home/seu_usuario_vm/minishell_proj
pwd - Imprimir Diretório de TrabalhoExibe o caminho completo do diretório de trabalho atual./> pwd
/home/seu_usuario_vm/minishell_proj
path [<diretorio1>:<diretorio2>...] - Gerenciar o PATHDefine ou limpa o PATH de pesquisa que o shell usa para encontrar executáveis externos. Se nenhum argumento for fornecido, ele limpa o custom_path, fazendo com que o shell volte a usar a variável de ambiente PATH padrão do sistema.Exemplo Detalhado:Crie um diretório temporário e um script executável de teste na sua VM (fora do seu shell, no terminal normal da VM):# Primeiro, saia do seu mini-shell (digite 'exit' e Enter)
# Volte para o terminal normal da VM
mkdir -p /tmp/meusbin
echo '#!/bin/bash' > /tmp/meusbin/meu_script.sh
echo 'echo "Olá do meu script personalizado na pasta temporária!"' >> /tmp/meusbin/meu_script.sh
chmod +x /tmp/meusbin/meu_script.sh
Isso cria um diretório /tmp/meusbin e um script meu_script.sh dentro dele, tornando-o executável.Inicie seu mini-shell novamente:./shell
Tente executar o script no seu shell (não funcionará ainda, pois o PATH do shell não inclui /tmp/meusbin):/> meu_script.sh
Command not found: meu_script.sh
Adicione /tmp/meusbin ao path do seu shell:/> path /tmp/meusbin
Execute o script novamente (agora deve funcionar, pois o shell sabe onde procurá-lo):/> meu_script.sh
Olá do meu script personalizado na pasta temporária!
Limpar o path personalizado (fazendo o shell voltar a usar o PATH do sistema):/> path
/> meu_script.sh
Command not found: meu_script.sh
Após limpar o path personalizado, seu shell não encontrará mais o script, pois ele não está nos diretórios padrão do sistema PATH.cat <arquivo1> [<arquivo2>...] - Concatenar e Exibir ArquivosExibe o conteúdo de um ou mais arquivos diretamente no terminal.# Crie arquivos de teste no seu shell
/> echo "Primeira linha." > arquivo1.txt
/> echo "Segunda linha." >> arquivo1.txt
/> echo "Conteúdo do arquivo dois." > arquivo2.txt

/> cat arquivo1.txt
Primeira linha.
Segunda linha.
/> cat arquivo1.txt arquivo2.txt
Primeira linha.
Segunda linha.
Conteúdo do arquivo dois.
ls [-a] [-l] [-al] - Listar Conteúdo do DiretórioLista os arquivos e diretórios dentro do diretório atual ou de um diretório especificado.-a: Exibe arquivos ocultos (aqueles que começam com um ponto .).-l: Exibe no formato longo, incluindo permissões, número de links, tamanho e outras informações.-al ou -la: Combina as opções -a e -l.# Crie alguns arquivos e diretórios para teste
/> touch .arquivo_oculto
/> touch arquivo_normal.txt
/> mkdir meu_diretorio

/> ls
arquivo_normal.txt  meu_diretorio

/> ls -a
.  ..  .arquivo_oculto  arquivo_normal.txt  meu_diretorio

/> ls -l
total 4
-rw-rw-r-- 1 seu_usuario_vm seu_usuario_vm   0 Jun  5 00:00 arquivo_normal.txt
drwxrwxr-x 2 seu_usuario_vm seu_usuario_vm 4096 Jun  5 00:00 meu_diretorio

/> ls -al
total 16
drwxrwxr-x 4 seu_usuario_vm seu_usuario_vm 4096 Jun  5 00:00 .
drwxrwxr-x 3 seu_usuario_vm seu_usuario_vm 4096 Jun  5 00:00 ..
-rw-rw-r-- 1 seu_usuario_vm seu_usuario_vm    0 Jun  5 00:00 .arquivo_oculto
-rw-rw-r-- 1 seu_usuario_vm seu_usuario_vm    0 Jun  5 00:00 arquivo_normal.txt
drwxrwxr-x 2 seu_usuario_vm seu_usuario_vm 4096 Jun  5 00:00 meu_diretorio
2. Comandos ExternosQualquer comando que não seja um dos built-ins mencionados acima será tratado como um comando externo. O shell tentará encontrá-lo no PATH configurado (seja o padrão do sistema ou o personalizado pelo comando path) e o executará como um novo processo./> echo "Olá do comando echo do sistema!"
Olá do comando echo do sistema!
/> date
Qui Jun  5 00:05:00 -03 2025
3. Redirecionamento de Saída ( > )Permite capturar a saída padrão de um comando e salvá-la em um arquivo, em vez de exibi-la no terminal. Se o arquivo não existir, ele será criado. Se existir, seu conteúdo será apagado antes da escrita./> ls -l > listagem_arquivos.txt
# Nenhum output no terminal, a saída foi para o arquivo.
/> cat listagem_arquivos.txt
total 4
-rw-rw-r-- 1 seu_usuario_vm seu_usuario_vm   0 Jun  5 00:00 arquivo_normal.txt
drwxrwxr-x 2 seu_usuario_vm seu_usuario_vm 4096 Jun  5 00:00 meu_diretorio
4. Pipes ( | )Permite encadear comandos, onde a saída padrão de um comando se torna a entrada padrão do próximo comando.# Este comando lista todos os arquivos, filtra as linhas que contêm "txt" e, em seguida, conta o número de linhas resultantes.
/> ls -a | grep txt | wc -l
2
5. Execução Concorrente ( & )O operador & permite que você execute vários comandos em paralelo. O shell iniciará cada comando em uma thread separada e esperará que todas as threads concluam antes de exibir um novo prompt./> sleep 3 & echo "Comando A terminou (depois de 3s)" & sleep 1 & echo "Comando B terminou (depois de 1s)"
# Você verá as mensagens aparecerem em ordem diferente, dependendo de qual comando termina primeiro.
# O prompt só aparecerá depois que ambos os 'sleep' e 'echo' forem concluídos.
Comando B terminou (depois de 1s)
Process exited with status: 0
Process exited with status: 0
Comando A terminou (depois de 3s)
Process exited with status: 0
Process exited with status: 0
/>
6. Execução de Arquivos BatchVocê pode passar um arquivo de texto contendo uma sequência de comandos para o seu shell executar de forma não interativa. Cada linha do arquivo é tratada como um comando.Crie um arquivo de exemplo (comandos_batch.sh):# Saia do seu mini-shell primeiro (digite 'exit' e Enter)
# No terminal normal da VM, na pasta do projeto:
echo "pwd" > comandos_batch.sh
echo "ls -l" >> comandos_batch.sh
echo "echo 'Fim da execução batch!'" >> comandos_batch.sh
echo "sleep 1 & echo 'Comando concorrente no batch'" >> comandos_batch.sh
Execute o shell com o arquivo batch:./shell comandos_batch.sh
A saída será similar a:Executing: pwd
/home/seu_usuario_vm/minishell_proj
Executing: ls -l
total 4
-rw-rw-r-- 1 seu_usuario_vm seu_usuario_vm   0 Jun  5 00:00 arquivo_normal.txt
drwxrwxr-x 2 seu_usuario_vm seu_usuario_vm 4096 Jun  5 00:00 meu_diretorio
Executing: echo 'Fim da execução batch!'
Fim da execução batch!
Executing: sleep 1 & echo 'Comando concorrente no batch'
Comando concorrente no batch
Process exited with status: 0
Process exited with status: 0
(Nota: As mensagens de "Process exited with status: 0" indicam que um processo filho ou thread foi concluído com sucesso. O número exato de mensagens pode variar dependendo dos comandos no batch e de como eles são tratados.)
