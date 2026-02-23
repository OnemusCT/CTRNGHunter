{% if rooms.twins > 0 %}{{<color "red">}}Do {$ rooms.twins $} extra sets of room transitions prior to the Masamune Twins battle.{{</color>}}
{% endif %}
**Masa and Mune**
{{<battlebox>}}Note: All attacks on Right twin

Lucca Hypnowave
Wait for Robo attack
Crono Attack
Wait for Robo to attack
Crono Attack
Lucca Fire
Crono Attack during Mune attack
Wait for Robo to attack
Lucca Fire
Crono auto-battle{{</battlebox>}}

**Masamune (3600 HP)**
{{<battlebox>}}
{% if rng.masamune == 1 -%}Auto-battle x6
Crono Mid-Potion Crono
Auto-battle x2
Crono Wind Slash
Auto-battle x6
Lucca Mid-Potion Crono
Auto-battle x3
Crono Wind Slash
Lucca Mid-Potion Robo
Auto-battle x5
Crono Wind Slash
Auto-battle{% else if rng.masamune == 35 -%}Auto-battle x9
Crono Wind Slash
Lucca Potion Crono
Auto-battle x5, wait for tornado
Lucca Potion Robo
Auto-battle x7 + Robo auto-attack
Crono Wind Slash
Lucca Potion Crono
Auto-battle{% else if rng.masamune == 37 -%}Auto-battle x9
Wind Slash
Potion Crono
Auto-battle x6
Crono Potion Crono
Auto-battle x5
Crono Wind Slash
Auto-battle x6
Lucca Potion Robo
Crono Wind Slash
Auto-battle x4{% endif %}{{</battlebox>}}