{% if heals.zombor == 0 -%}
Use Potions to heal before the battle.
{% else -%}
{{<color "red">}}Perform {$ heals.zombor $} Tech heals prior to the Zombor battle. Tech heals will advance RNG even if the target is full health.{{</color>}}{% endif -%}
{% if heals.zombor < 3 and heals.zombor != 0 %}
Use Potions for additional heals beyond the required {$ heals.zombor $}{% endif %}

**Zombor**
{{<battlebox>}}Robo auto attacks butt for 80 if on manip
Crono Cyclone
Lucca Attack head
Crono Cyclone
Robo auto head crit
Lucca Attack head
Crono Cyclone
Lucca Attack head
Robo auto head crit 
Crono Wind Slash
Lucca Fire
Robo auto
Crono Wind Slash
Lucca Fire{{</battlebox>}}