Use Tech heals once on each character.
{% if "flea" in heals and heals.flea > 0 -%}
	{{<color "red">}}Perform {$ heals.flea $} additional Tech heals prior to the Flea? battle. Tech heals will advance RNG even if the target is full health.{{</color>}}
{% endif %}
{% if "flea" in rooms and rooms.flea > 0 -%}
	{{<color "red">}}Do {$ rooms.flea $} extra sets of room transitions prior to the Flea? battle.{{</color>}}
{% endif %}
**Fake Flea**
{{<battlebox>}}
Frog Attack (MP Buster){{</battlebox>}}
{% set auto=[1,2,3,11,12] %}{% set counter-potion=[9,10] %}
**Flea (4120 HP)**
{{<battlebox>}}
{% if "flea" in rng and rng.flea in auto -%}
Note: Robo and Crono should be full HP, Frog should be <210 HP but >100
Auto-battle to end{% else if "flea" in rng and rng.flea in counter-potion -%}
Auto-battle until counter
Crono potion Crono
Auto-battle to end{% else -%}
Auto-battle x13
Robo attack
Crono mid-potion Crono
Auto-battle x8
Crono mid-potion Crono
Auto-battle to end{% endif -%}{{</battlebox>}}