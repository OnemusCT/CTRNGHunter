{% if rooms.yakra > 0 %}Do {$ rooms.yakra $} extra sets of room transitions prior to the Yakra battle. Remember you can't exit Yakra's room once you enter
{% endif %}
**Battle**
{{<battlebox>}}
{% if rng.yakra >= 3 and rng.yakra <= 8 %}* Auto battle x6 attacks  
Wait until Needles  
Auto-battle to end
{% else if rng.yakra >= 62 and rng.yakra <= 87 %}
Note: Lucca HP >= 54, full health is fine



Auto-battle x3  
Crono attack  
Wait until Needles  
Auto-battle to end
{% else if (rng.yakra >= 9 and rng.yakra <= 12) or (rng.yakra >= 152 and rng.yakra <= 158) or (rng.yakra >= 161 and rng.yakra <= 163) %}* Auto battle
{% else if rng.yakra >= 88 and rng.yakra <= 96 %}* Auto battle x3  
Wait until Needles  
Auto-battle to end{% endif %}{{</battlebox>}}