# Display-com-alerta-MQTT
Display com alerta MQTT para enviar avisos via Home Assistant

Comandos (Final da frase):

  "?" -> Pergunta, sem alerta sonoro. Respostas (SIM, NAO). Ex: "Tudo bem?"
  
  "??" -> Pergunta, com alerta sonoro. Respostas (SIM, NAO). Ex: "Tudo bem??"

  "!" -> Comando de Ação, sem alerta sonoro. Respostas (OK). Ex: "Vai tomar banho!"
  
  "!!" -> Comando de Ação, com alerta sonoro. Respostas (OK). Ex: "Vai tomar banho!!"

  A cada 5 segundos, quando tem alerta sonoro, o som se repete por 3 vezes, apos isso, retorna automaticante uma msg "AUSENTE" e volta para o relogio.

  Se nâo tem alerta sonoro, conta o tempo 5 segundo por 3 vezes e retorna automaticante uma msg "AUSENTE" e volta para o relogio. 
  

  Se não tiver nenhum comando, ex: "Bom dia :)" a msg é exibida no display sem nenhum alerta, e é sustituida quando atulizar o horario.

  
  

![Painel Foto LCD](https://github.com/diego123cruz/Display-com-alerta-MQTT/blob/main/Foto.jpeg)

