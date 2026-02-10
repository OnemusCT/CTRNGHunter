{% if rooms.dragontank > 0 %}{{<color "red">}}Do {$ rooms.dragontank $} extra sets of room transitions prior to the Dragon Tank battle.{{</color>}}
{% endif %}
**Battle**
{{<battlebox>}}Note: Two of Crono's attacks should be crits if on manip

Crono Attack Head
Lucca Attack Head
Crono Attack Head
Lucca Auto-attack (attack body)
Crono Attack Head
Crono Cyclone
Lucca Potion {% if rng.dragontank in [255,1] %}Crono{% else %}Lucca{% endif %}
Crono Cyclone
Lucca Fire Whirl
Crono Fire Whirl{{</battlebox>}}