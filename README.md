# Best-Beter-Meter

Best Beter Meter
CO2 en temperatuur monitor


Versie:					03
Datum:				15-9-2019
Ontwikkeld door:		Sander Claassen
Ontwikkelbron:		https://github.com/SANCLA/Best-Beter-Meter

Inhoud
1. 	Introductie Best Beter Meter
2. 	Functies en werking
3. 	CO2 concentraties
4. 	Registratie op SD kaart
5. 	Bronnen


1. Introductie Best Beter Meter

Op kantoor is er soms de behoefte om de luchtkwaliteit inzichtelijk te maken daar deze van grote invloed is op de werkprestaties en algehele gezondheid van aanwezigen. De Best Beter Meter geeft een invulling aan door een van de belangrijkste aspecten te meten, inzichtelijke te maken en te registreren. 
De Best Beter Meter monitort de CO2 en temperatuur waarden en maakt deze toegankelijk inzichtelijk middels een stoplicht feedback. Daarnaast is er, door de registratie op een geheugenkaart, ook een lange termijn trend inzichtelijk te maken.  

Hoewel de luchtkwaliteit door veel meer aspecten beinvloed kan worden dan alleen CO2 en temperatuur, zijn deze waarden relatief eenvoudig te meten en veelal een aanwegezen indicatie van de algehele luchtkwaliteit. 

2. Functies en werking

De Best Beter Meter meet elke 10 seconden de CO2 en de temperatuur waarden. 
Een gemiddelde van 3 metingen wordt verwerkt, geregistreerd en weergegeven op het LCD display. De Best Beter Meter heeft een SD kaart nodig om te kunnen functioneren.

Elke 5 minuten worden deze gemiddelde waarden weggeschreven, inclusief tijd en datum, naar de SD geheugenkaart in een CSV (Comma Seperated value) bestand welke in bijvoorbeeld Microsoft Excel kan worden ingelezen.

De Best Beter Meter heeft een 4-tal LED’s waarmee de status en de CO2 concentraties kunnen worden afgelezen, alsmede een LCD display waarop exacte waarden en status zichtbaar zijn.

LED Status:

LED     Status                        Resultaat

Blauw   Continue                      Fout, zie LCD display…

Blauw   Knippert kort (1 sec)         Intialiseren (duurt ca 30 seconden)

Blauw   Knipprt zeer kort (100 ms)    Wegschrijven meeting naar SD geheugenkaart

Blauw   Knippert lang (3 sec)         Meting  CO2 en temperatuur waarde

Groen   Continue                      CO2 waarde lager dan 1000 PPM

Geel    Continue                      CO2 waarde tussen de 1000 PPM en de 1500 PPM

Rood    Continue                      CO2 waarde hoger dan 1500 PPM

3. CO2 concentraties

De World Health Organisation (WHO) heeft helaas geen aanbevelingen gepubliceerd over CO2 concentraties binnenshuis en de effecten hiervan. Wel een publicatie voor het tot stand komen van een algemene standaard [1], echter is het WHO hier nog niet zover mee. De WHO heeft enkele aanbevelingen en publicaties op haar website gepubliceerd op een aantal andere facetten van luchtkwaliteit. De Nederlandse overheid heeft hier echter niet concreets gepubliceerd, anders dan dat er voldoende luchtverversing plaats dient te vinden [3].

Binnen Europa is de standaard maximaal 3.500 PPM blootstelling op een normale werkdag. Binnen het Verenigd Koninkrijk is het advies voor scholen een maximale bloodstelling van 1.500 PPM. De Verenigde state adviseerd als maximaal blootstelling een waarde van 5.000 PPM. Het varieert nogal dus wat goed en wat slecht is. Wel is iedereen er in grote over eens dat het goed is om te gaan ventileren bij waarden boven de 1.000 PPM [2]. 

Ter indicatie kan men de weergegeven tabel aanhouden als maatstaaf voor het beoordelen van de metingen en waarden. Het LED stoplicht systeem is hierop gebasseerd.

4. Registratie op SD kaart

Elke keer als de Best Beter Meter wordt aangesloten zal deze, omhoog nummerend, een nieuw registratiebestand aanmaken genaamd “LOGGER001.CSV”, waarbij het nummer oploopt. Tot 100 registratiebestanden worden automatisch aangemaakt, daarna wordt de eerste weer overschreven. Elke 5 minuten wordt er een registratie naar de SD kaart weggeschreven.

In het registratiebestand worden de navolgende zaken in CSV (Comma Seperated Value) formaat opgenomen:

Veld        Voorbeeld             Omschrijving

msrunning   1237064431            Aantal miliseconden dat de Best Beter Meter actief is sinds de laatste herstart

unixtime    1568559342            Datum/Tijd van registratie in het UNIX time format

datetime    2019/9/15 14:54:31    Datum/Tijd van registratie in het volgende format:

                                  jaar/maand/dag uur:minuten:seconden
                                  
co2uart     988                   CO2 waarde (gemiddelde van 3 metingen)

temp        21                    Temperatuur waarde (gemiddelde van 3 metingen)


De SD kaart dient in het FAT16 of FAT32 bestandsformaat geformateerd te zijn.
Als tijdens het actief zijn van de Best Beter Meter de SD kaart onverwacht wordt verwijderd kan er corruptie onstaan in het registratiebestand en zal de registratie van metingen op de SD kaart stoppen totdat de Best Beter Meter opnieuw wordt opgestart. 

5. Bronnen

[1]	http://www.euro.who.int/__data/assets/pdf_file/0007/78613/AIQIAQ_mtgrep_Bonn_Oct06.pdf
[2]	https://en.wikipedia.org/wiki/Carbon_dioxide
[3]	https://www.arboportaal.nl/onderwerpen/luchtverversing
