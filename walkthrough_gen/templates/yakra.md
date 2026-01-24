# Yakra

Battle Mode: Wait  
Name Crono

## Crono’s House

**Menu**  
Settings

* Battle Speed: 3  
* Battle Message Speed: 1  
* Battle Cursor Memory: Full

Name Lucca (Down, Confirm, Up, Confirm)  
**Get 200 G** (talk to Mother again)

## Leene Square

Bump into Marle, **talk to her again** then grab the pendant  
Name Marle (Down, Confirm, Up, Confirm)  
Back to fountain and talk to the purple tent vendor

**Shop**  
Buy

* Potion x11  
* Shelter x3

Go all the way north, wait on candy scene  
Talk to Lucca then to Marle (don’t test the telepod)  
Hold Up+Left and get the pendant

## Truce Canyon

Hold Left

**Blue Imp x3**

* Attack Bottom-Right (Confirm, Right, Confirm)  
* Auto Battle

Perform Bird Skip

* Image shows 1st pixel  
* Align Crono’s foot with edge then run up

![][image1]

**Pick Power Glove**

## Guardia Forest

Go right and stay down to avoid fight  
**Pick Strength Capsule**  
Go north to castle

## Guardia Castle

Go to throne room, right path, upstairs (4 screen transitions)  
Talk to Marle in her room  
**Pick Ether** in Marle’s room  
Back to forest

## Guardia Forest

**Menu**  
Equipment

* Crono (Wooden Sword / Hide Cap / Hide Tunic / **Power Glove**)  
  * Power Glove  
* Lucca (Airgun / Hide Cap / Padded Vest / **Headband**)  
  * Headband

Inventory

* Potion ↔ Shelter (Right)  
* Strength Capsule (Left+Down) → Crono

**Roundillo Rider x2**

* Auto Battle

**Roundillo Rider x3**

* Cyclone all (Right)

## Cathedral

**Menu**  
Inventory

* Shelter

**Naga x4**

* Fire Whirl \+ Flamethrower if Fire Whirl hit 3  
* Fire Whirl x2 otherwise

Name Frog (Down, Confirm, Up, Confirm)  
Play the organ and go to next room

**Pick**

* Athenian Water (right chest, optional \- just hold left if won’t pick)  
* Steel Saber (top left chest)  
* Strength Capsule (top left room)

Go to next room and clockwise

* Ignore left skull (just hold up if didn’t waste any movement)  
* At top, stay down whole time (won’t get encounter if on right cycle)  
* Push right skull, go down avoiding bat (Diablos have no hitbox)

**Underling x3, Diablo x2**

* Crono and Frog Attack Diablo (Left x2)  
* Lucca Fire Whirl Underlings  
* Auto Battle

**Pick Iron Sword**

Play the organ and go to the next room (don’t fall on center of stairs, stay right or left)

**Menu**  
Equipment

* Crono (**Steel Saber** / Hide Cap / Hide Tunic / Power Glove)  
  * Steel Saber  
* Frog (**Iron Sword** / Bronze Helm / Bronze Armor / Power Glove)  
  * Iron Sword

Inventory

* Strength Capsule → Crono  
* Ether → Crono

Tech

* Frog  
  * Slurp Frog and Crono

Go up and between last enemies to next screen

**Yakra**
{% if rng.yakra >= 3 and rng.yakra <= 8 %}
* Auto battle x6 attacks  
* Wait until Needles  
* Auto battle to end
{% else if rng.yakra >= 62 and rng.yakra <= 87 %}
* Auto battle x3  
* Crono attack  
* Wait until Needles  
* Auto battle to end
{% else if (rng.yakra >= 9 and rng.yakra <= 12) or (rng.yakra >= 152 and rng.yakra <= 158) or (rng.yakra >= 161 and rng.yakra <= 163) %}
* Auto battle
{% else if rng.yakra >= 88 and rng.yakra <= 96 %}
* Auto battle x3  
* Wait until Needles  
* Auto battle to end
{% endif %}

**Pick Mid-Ether** (right chest)

