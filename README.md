# Digitech Whammy 5 MIDI Controller With Bluetooth Interface
**_NOTE:_**  The code is not finilized. It working but has known flaws that will be worked on on future iterations.

This is a MIDI controller for the Digitech Whammy 5/ Bass with an ESR32-Cam and an accompanying app made with MIT app inventor
The ESP32-Cam was selected for its compactness and connection capabilities.
With the app you can select the programs you want to loop through in the two modes of the pedal, classic and chords. Also you can select if the programs will go on active or bypass mode.
The two lists stay saved on the device and you need to connect only to update your list.

## Connection Process
Switch the devise to pairing mode and select it from the available list on the top of the app.
The interface will update to show the current programs in the list. For the selected mode(classic/chords) from the switch select the programs you want and the bypass checkbox option and press the apply button.
If everything executes correctly an appropriate prompt will appear.

<img src="Images/classic.png" width=20% height=20%>)
<img src="Images/chords.png" width=20% height=20%>)

## Playing
While not in pairing, the mode switch controls what array you are going to loop classic or chords. When in normal sequence, the led_norm will be on and the order will be ascending (or clockwise on the pedal) when the foot button is pressed. If the foot button is pressed for 0.5 sec it will go in reverse mode and the led_rev will light on. The array will be descending (or counterclockwise on the pedal).

## App
To get the app download the .apk file and install it. To modify the app, download the .aia file and import it to MIT App Inventor.


##Schematic 

<img src="Images/Schematic.png" width=50% height=50%>)



