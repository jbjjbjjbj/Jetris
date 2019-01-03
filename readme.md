# Julian Mini Tetris spil

En lille udgave af de klassiske spil Tetris udgivet i 1984(src. wikipedia). Spillet går ud på at spilleren skal få forskellige brikker til at passe sammen.
Disse brikker falner ned oven fra og spilleren skal arrangere dem inden de rammer noget. 

Modsat normal tetris giver antallet af rækker man klarer på en gang ikke flere point. 
Dette ødelægger lidt hele ideen med spillet og vil give mening til en version 2. 

## Kode teknisk 

Koden er delt op i forskellige filer

**Generelt:** (Indeholder setup, input og gamelogic)

- Jetris.ino

**Tegne funktioner:**

- drawing.h
- drawing.ino

**Brikker og figurer:**

- sprites.h
- sprites.ino

**Definitioner:** (indeholder registernavne til skærm)

- maxCommands.h

For at holder styr på hvad der er på skærmen bruges et array med bytes som har længde 16. 
Grunden til at der bruges bytes til de horisontale prikker er fordi det gør manipulation meget let. 

For at tegne figurer skrives der til dette array og ændringer sendes ud til skærmen. 
Dette betyder at det let kan implementeres på andre skærme, fx. der er en compiler flag der giver mulighed for print til seriel.  


## Problemer og Forbedringer

- Der er ligenu en del problemer med forbindelse til skærmen, hvilket betyder at der er prikker tit bliver lyst forkert op. 
Derfor bliver hele skærmbufferen sendt forfra 4 gange i sekundtet
- Måden kommandoer bliver sendt til skærm er ikke særlig effektiv. 
- Scoring/level system er ikke særlig godt


