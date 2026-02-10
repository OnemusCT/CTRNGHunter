+++
title = 'Chrono Trigger PC - 100% RNG Manip'
norss = true
showDate = false
description = 'Simplified walkthrough of the manips for the CT 100% RNG Manip (PC) route'
+++
<!-- 
The format of these templates is catered towards onemus.net.
If you aren't me I'm sorry.
-->
{% if rooms.cathedral-trash > 0 %}{{<color "red">}}Do {$ rooms.cathedral-trash $} extra sets of room transitions prior to the battle with 3 Hench and 2 Gargoyles.{{</color>}}
{% endif %}## Yakra
{% include "yakra_heal" %}
{% include "yakra_battle" %}

## Dragon Tank
{% include "dragontank_battle" %}

## Zombor
{% include "zombor_rooms" %}
{% include "zombor_battle" %}

## Masamune
{% include "masamune_battle" %}

## Nizbel
{% include "nizbel_battle" %}

## Flea
{% include "flea_battle" %}

## Magus
{% include "magus_battle" %}