{% if rooms.dragontank > 0 %}{{<color "red">}}Do {$ rooms.dragontank $} extra sets of room transitions prior to the Dragon Tank battle.{{</color>}}
{% endif %}
**Dragon Tank**
{{<battlebox>}}Note: Two of Crono's attacks should be crits if on manip

Wait for Head to heal
Crono Attack Head RIGHT
Lucca Attack Head RIGHT
Crono Attack Head RIGHT
Lucca Auto-attack (attack body)
Crono Attack Head RIGHT
Crono Cyclone
Lucca Potion {% if rng.dragontank in [255,1] %}Crono{% else %}Lucca{% endif %}
Crono Cyclone
Lucca Fire Whirl
Crono Fire Whirl{{</battlebox>}}