{% if "flea" in rooms and rooms.flea > 0 -%}
	{{<color "red">}}Do {$ rooms.flea $} extra sets of room transitions prior to the Flea? battle.{{</color>}}
{% endif %}
**Fake Flea**
{{<battlebox>}}Frog Attack (MP Buster){{</battlebox>}}
{% set auto=[1,2,3,11,12] %}{% set counter-potion=[9,10] %}
**Flea (4120 HP)**
{{<battlebox>}}{% if existsIn(rng, "flea") and rng.flea in auto -%}Note: Robo and Crono should be full HP, Frog should be <214 HP but >100
Auto-battle to end

BACKUP if Frog > 214 HP
Auto-battle 5 frog attacks (+1 queued Crono attack)
Robo Mid-Potion Crono during counter
Auto-battle until next counter
Frog attack
Auto-battle{% else if "flea" in rng and rng.flea in counter-potion -%}
Auto-battle until counter
Crono potion Crono
Auto-battle to end{% else -%}
Auto-battle x13
Robo attack
Crono mid-potion Crono
Auto-battle x8
Crono mid-potion Crono
Auto-battle to end{% endif -%}{{</battlebox>}}