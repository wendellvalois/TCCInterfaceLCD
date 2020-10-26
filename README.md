# TCCInterfaceLCD

Interface Arduino direta para coleta de dados evitando a dependência de um laptop ou computador por perto.

O código principal do Arduino encontra-se na página <i>src</i>. Copie o conteudo para um arquivo .ino para funcionar na IDE padrão do Arduino se for neccessário.
  
Foi utilizado Visual Studio Code + PlatformIO. Aconselho a usar para projetos mais complexos, como este. Embora tenha alguns inconvenientes de configuração dos quais não cobrirei.  


## Interface de apresentação de gráficos
Para o tratamento de dados coletados foi utilizado o Python 3 e para interface gráfica foi usada biblioteca Tkinter já inclusa
em muitas versões do Python. É necessário a instalação de matplotlib para geração dos gráficos.  

O Python foi escolhido pela facilidade e por ser multiplataforma.
Nada impede a criação de interfaces de apresentação de dados em outras linguagens de programação como linguages web por exemplo.

