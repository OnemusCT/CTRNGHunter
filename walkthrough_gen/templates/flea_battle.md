{% if rooms.flea > 0 -%}
	Do {$ rooms.flea $} extra sets of room transitions prior to the Flea? battle.
{% endif %}

Use Tech heals once on each character.
{% if heals.flea > 0 -%}
	Perform {$ heals.flea $} additional Tech heals prior to the Flea? battle. Tech heals will advance RNG even if the target is full health.
{% endif %}
{% set auto=[1,2,3,11,12] %}{% set counter-potion=[9,10] %}
**Battle**
{{<battlebox>}}
{% if rng.flea in auto -%}
Auto-battle to end{% else if rng.flea in counter-potion -%}
Auto-battle until counter
Crono potion Crono
Auto-battle to end{% else -%}
Auto-battle x13
Robo attack
Crono mid-potion Crono
Auto-battle x8
Crono mid-potion Crono
Auto-battle to end{% endif -%}{{</battlebox>}}