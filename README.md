# 1. Was macht unser Projekt?

Unser Projekt "Space Invaders" ist ein eigenständig entwickeltes Arcade-Spiel, das aus zwei ESP32-Geräten besteht und ein farbiges LED-Matrix als Anzeige verwendet. Die zentrale Idee hinter dem gesamten Projekt war es, zwei ESP32 Geräte miteinander über das MQTT-Protokoll zu verbinden, so dass sie miteinander kommunizieren können.

Die Funktionalität des Spiels wurde dabei strikt getrennt: Ein ESP32 fungiert ausschließlich als Joystick-Controller. Dieses Gerät ist dafür zuständig, die analogen Werte beider Achsen (X, Y), sowie den Feuerknopf kontinuierlich auszulesen und an den MQTT-Broker zu senden. Der andere ESP32 übernimmt die gesamte Anzeige des Spieles und hat die Aufgabe, die Steuerdaten des Joysticks in Echtzeit zu empfangen.

Die erfassten Steuerdaten werden regelmäßig über WLAN an den MQTT-Server gesendet. Dieser Server dient als zentraler Vermittler und leitet die Daten an das Anzeige-ESP32 Gerät weiter.

Das Anzeige-ESP32 Gerät nutzt die empfangenen Steuerbefehle, um daraus die neue Position des Spielers zu berechnen. Danach berechnet das Gerät das Verhalten der Gegner sowie die exakte Position der Schüsse auf dem Spielfeld und prüft auf Kollisionen. Die fertige, gerenderte Szene wird dann Bild für Bild in einem dedizierten Framebuffer im Speicher aufgebaut. Anschließend wird dieser Framebuffer-Inhalt auf dem Pixeldisplay wiedergegeben.

Das laufende Spiel erzeugt den Eindruck eines kleinen, voll funktionsfähigen Spielautomaten. Der Spieler steuert sein Raumschiff, bewegt sich in verschiedene Richtungen und kann auf die Gegner feuern. Im Hintergrund der Anwendung werden durchgehend Daten ausgelesen, Nachrichten übertragen, Kollisionen berechnet und grafische Frames überzeugt um ein flüssiges Spielerlebnis zu ermöglichen. Diese Architektur demonstriert somit eine klare Trennung von Eingabe, Logik und Ausgabe über eine Netzwerkverbindung, was ein zentrales Lernziel des Projekts darstellt.

# 2. Aufbau des Projekts (Verkabelung und Komponenten)

Das gesamte System besteht aus zwei voneinander getrennten ESP32-Geräten, die über dasselbe WLAN-Netzwerk miteinander kommunizieren. Diese sind das Joystick-ESP32 Gerät als reine Eingabeeinheit und das Anzeige-ESP32 Gerät mit dem farbigen Pixeldisplay, welches für die Logik und Ausgabe zuständig ist.

Der Joystick-ESP32 ist fest mit einem analogen 2 Achsen Joystick verbunden. Die X-Achse und die Y-Achse des Joysticks werden direkt an die analogen Pins des ESP32 angeschlossen. Durch diese direkte Verbindung können die Steuerungswerte in Form von Spannungswerten kontinuierlich und präzise gelesen werden. Der Feuerknopf des Joysticks ist zusätzlich an einem digitalen Pin angeschlossen. Hier ist auch eine kleine Status-LED vorhanden, die den Zustand des Feuerknopfs visuell anzeigt. Der Joystick-Controller ist damit die einzige Quelle für die Eingabebefehle im ganzen System.

<img width="300" src="https://github.com/user-attachments/assets/c0a5c937-bc32-456a-b0d9-89bf3eea2804" />

Der Anzeige-ESP32 ist durch viele einzelne Kabel direkt mit dem farbigen Pixeldisplay verbunden. Diese Kabel sind notwendig, um alle nötigen Bild- und Steuerbefehle an die Matrix zu übertragen.

Des Weiteren sind zwei zusätzliche lokale Tasten am Anzeige-ESP32 verbunden. Einer der beiden Tasten positioniert die Gegner neu auf dem Spielfeld und die andere Taste lässt das Spiel manuell komplett neustarten, falls es beispielweise zu einem Fehler kommt. Bei dieser Einheit ist die Stromversorgung sehr wichtig. Das Display muss mit au0sreichend Strom versorgt werden, damit die LED-Matrix problemlos angesteuert werden kann.

<img width="300" src="https://github.com/user-attachments/assets/4805cfe8-3088-48ca-b1b8-c302031fe2e9" />

Beide ESP32-Geräte sind über das gleiche WLAN-Netz mit dem MQTT-Broker, der über den Laptop läuft, verbunden. Das folgende Bild zeigt diesen Gesamtaufbau, in dem der Laptop als zentrale Instanz für den Broker dient.

<img width="600" src="https://github.com/user-attachments/assets/1b41fa03-b5b9-482a-9346-1297df3cee07" />

Der Joystick-ESP32 meldet sich als Client auf dem MQTT-Server an und sendet (publiziert) die Joystickdaten. Diese Joystickdaten sind der wichtigste Datenstrom im System. Währenddessen meldet sich der Anzeige-ESP32 sich auch an, um die Daten zu empfangen (abonniert) und sofort in die Spielelogik einfließen zu lassen. Das ganze System basiert auf dieser Netzwerkverbindung. Auf folgendem Bild ist das fertige laufende Spiel zu sehen.
 
<img width="600" src="https://github.com/user-attachments/assets/c96cdc70-9ecc-4ec5-8a30-0c3707f09461" />


# 3. Erläuterung und Beschreibung des Codes

Der Softwareaufbau ist modular gestaltet, damit die verschiedenen Funktionen klar getrennt und besser wartbar sind. Beide ESP32-Programme verwenden das ESP-IDF-Framework als Einstiegspunkt.

Zu Beginn initialisiert das Programm den nichtflüchtigen Speicher startet das Standard-Event-Loop und konfiguriert den WLAN-Treiber im Station-Modus. SSID und Passwort kommen aus den Konfigurationswerten. Sobald von dem DHCP-Server eine IP-Adresse zugewiesen wurde, wird die MQTT-Verbindung durchgeführt.

Für die MQTT-Kommunikation wird der ESP-IDF MQTT-Client verwendet. In einer Konfigurationsstruktur werden Host (IP des Laptops) Port (1883) Client-ID sowie weitere Parameter gesetzt. Mit den Befehlen esp_mqtt_client_init und esp_mqtt_client_start wird der Client gestartet und ein Event-Handler registriert. Der Event-Handler reagiert auf Ereignisse wie „verbunden“ „getrennt“ oder „neue Nachricht“. Über EventGroups wird die Netzwerk- und Brokerverbindung überwacht, so dass die Spiellogik erst startet, wenn eine stabile Verbindung vorhanden ist.

Auf dem Joystick-ESP32 kümmert sich ein Eingabemodul um die Konfiguration der ADC-Kanäle und das Einlesen der Rohwerte der X- und Y-Achse. Der Button-Status wird parallel gelesen. Diese Werte werden in einer kompakten Struktur zusammengefasst und in Abständen von etwa 200 Millisekunden über den MQTT-Client als binärer oder einfacher serialisierter Datenblock auf das Topic /js gesendet. Die Status-LED wird dabei abhängig vom Button-Zustand geschaltet, was beim Debugging hilft.

Der Anzeige-ESP32 enthält neben der MQTT-Verarbeitung ein Button-Modul für die lokalen Taster und eine Grafikschicht. Die Grafikschicht stellt einen Framebuffer und eine Sprite-Bibliothek bereit. Der Framebuffer belegt einen definierten Speicherbereich im RAM, wo für jedes Pixel RGB-Werte gespeichert werden. Hilfsfunktionen ermöglichen das Löschen des Framebuffers das Zeichnen von Sprites an bestimmten Positionen und das Berechnen einfacher Farbmuster.

Die Spiellogik selbst ist in einer Game-Struktur organisiert. Die Game-Struktur enthält das Spielerschiff mit Position Lebenspunkten und Sprite ein Array von Gegnern sowie eine Bullet-Entität für die Schüsse. Zu Beginn initialisiert eine Funktion alle Startwerte und ordnet die Gegner in Reihen auf dem Spielfeld an.

Im Hauptloop des Anzeige-ESP32 läuft ein fester Zyklus ab: Zuerst werden die WLAN- und MQTT-Verbindungsbits geprüft. Falls Probleme bestehen, zeigt das Display Fehlermeldungen an und überspringt die Spielberechnung. Danach versucht der Code eine neue Joystick-Messung aus einer Queue zu lesen, ohne zu blockieren und liest zusätzlich die lokalen Taster. Aus den Rohwerten werden einfache Flags wie links rechts und fire abgeleitet mit denen das Schiff gesteuert und ein Schuss ausgelöst wird. Die Position des Projektils wird in jedem Frame aktualisiert und auf Kollisionen mit Gegner-Rechtecken geprüft. Trifft ein Projektil einen Gegner wird dieser deaktiviert der Score erhöht und das Projektil ausgeschaltet.

Für das Rendering wird zuerst der Framebuffer gelöscht dann werden Spieler Projektile und Gegner auf den Framebuffer gezeichnet und der aktuelle Score als Text eingeblendet. Anschließend läuft das Programm über alle Pixelkoordinaten liest die RGB-Werte aus dem Framebuffer und übergibt diese an die HUB75-Treiberfunktionen, die die LED-Matrix aktualisieren. Zwischen zwei Frames sorgt ein vTaskDelay dafür, dass sich eine Bildrate von etwa 20 Frames pro Sekunde ergibt. Trotz der begrenzten Ressourcen des Mikrocontrollers entsteht so eine flüssige Spielanimation während im Hintergrund Netzwerk MQTT-Events Eingabe und Spiellogik parallel ablaufen.

# 4. Welche Probleme bei der Projektrealisierung aufgetreten sind

Während der Umsetzung des Projekts sind verschiedene Probleme aufgetreten. Die Schwierigkeiten lagen hauptsächlich im praktischen Aufbau und der Netzwerkstabilität.

Ein häufiges Problem war die Verkabelung am LED-Display. Der Anschluss (HUB75-Stecker) besitzt viele Pins, die sehr eng beieinander liegen. Schon wenn ein Kabel nur um eine Position verschoben wurde, gab es entweder gar kein Bild nur ein zufälliges Flackern oder die Farben wurden falsch dargestellt.

Anfangs wirkte der Code korrekt, aber die Anzeige war fehlerhaft. Der Fehler konnte erst durch systematisches Testen gefunden werden. Dazu mussten wir das Pinout (die Belegungs-Anweisung) genau mit der Dokumentation vergleichen und kleine Testprogramme schreiben (zum Beispiel nur Text Anzeigen). Am Ende zeigte sich wie wichtig eine saubere Beschriftung und eine ordentliche Kabelführung bei so vielen Leitungen ist.

Ein weiteres zentrales Problem war die Qualität der WLAN-Verbindung. Da das ganze System die Steuerdaten über MQTT schickt, führte jede schlechte Verbindung sofort zu einer spürbaren Verzögerung. Joystick-Bewegungen kamen verzögert an Schüsse wurden nicht registriert oder die Verbindung zum Broker brach ganz ab.

Zum Teil lag das an der Position des Routers oder die Anzahl von Geräten im Netzwerk, zum Teil aber auch an Energiesparfunktionen im ESP32-WLAN-Treiber. Um das zu verbessern haben wir den WLAN-Power-Save-Modus im ESP32 deaktiviert. Zusätzlich wurde der MQTT-Client so eingestellt, dass er sich bei Verbindungsverlusten automatisch neu verbindet und auf der Matrix sofort klare Fehlermeldungen wie „WIFI DISCONNECTED“ oder „MQTT DISCONNECTED“ anzeigt.

<img width="600" src="https://github.com/user-attachments/assets/5b108235-4d34-4893-8a93-a3b2fdeb5c8f" />

Des Weiteren gab es Probleme bei der Mittelstellung des analogen Joysticks. Analoge Joysticks liefern selten den genauen Nullwert, sondern zeigen leichte Abweichungen und Rauschen. Bei uns führte das dazu, dass sich das Schiff leicht von alleine bewegte obwohl der Joystick in der Mitte stand.

Wir haben dieses Verhalten durch eine einfache Kalibrierroutine behoben. Es wurde eine Dead-Zone (ein kleiner Toleranzbereich) um die Mittelstellung eingeführt. Dadurch wird keine Bewegung ausgelöst, wenn der Joystick nur leicht abweicht, was die Steuerung deutlich stabiler gemacht hat.


# 5. Fazit und Ausblick

Das Projekt hat gezeigt, dass sich klassische Mikrocontroller-Themen wie analoge Eingabe, digitale Taster, Framebuffer-Grafik und die Echtzeit-Spiellogik gut kombinieren lassen. Wir haben dabei WLAN und das MQTT-Protokoll erfolgreich genutzt. Die Aufteilung in einen separaten Joystick-Controller und eine Anzeigeeinheit ist wichtig, weil sie praxisnah vermittelt, wie verteilte Systeme überhaupt aufgebaut werden. Es zeigt auch, wie ein Client-Server-Umgang mit Eingabegeräten umgesetzt werden kann.

Die Probleme, die während der Entwicklung aufgetreten sind, waren lehrreich und führten zu vielen Verbesserungen. Die Verkabelung wurde robuster gestaltet, das WLAN wurde stabiler eingerichtet und die Software hat zusätzliche Mechanismen für die Fehleranzeige und das automatische Reconnect bekommen. Durch das systematische Beheben dieser Fehler ist das gesamte System nun deutlich stabiler geworden und funktioniert zuverlässig.

Für zukünftige Ausbaustufen sind noch mehrere Verbesserungen möglich. Dazu gehört eine manuelle Kalibrierungsroutine für Joysticks, damit diese genauer sind und auch bei anderen Modellen keine ungewollte Bewegung entsteht. Wir könnten auch eine erweiterte Fehlerbehandlung mit Logging einführen, um Probleme besser zu protokollieren. Außerdem können zusätzliche Spielmodi und Animationen auf der Matrix eingebaut werden. 
