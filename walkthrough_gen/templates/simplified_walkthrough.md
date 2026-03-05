<!-- 
The format of these templates is catered towards onemus.net.
If you aren't me I'm sorry.
-->
## Load Seed: {$ seed $} ({$ seedString $})

{%- if existsIn(rng, "yakra") -%}
{% if rooms.cathedral-trash > 0 %}{{<color "red">}}Do {$ rooms.cathedral-trash $} extra sets of room transitions prior to the battle with 3 Hench and 2 Gargoyles.{{</color>}}
{% endif %}
{% include "yakra_heal" %}
{% include "yakra_battle" %}

{%- endif -%}
{%- if existsIn(rng, "dragontank") -%}
[Backup](/speedruns/ct/rng-manip-100-simple/backups/dragontank/)

{% include "dragontank_battle" %}

{% include "guardian_battle" %}
{%- endif -%}
{%- if existsIn(rng, "zombor") -%}
{% include "zombor_rooms" %}

[Backup](/speedruns/ct/rng-manip-100-simple/backups/zombor/)

{% include "zombor_battle" %}

{%- endif -%}
{%- if existsIn(rng, "masamune") -%}
[Backup](/speedruns/ct/rng-manip-100-simple/backups/masamune/)

{% include "masamune_battle" %}

{%- endif -%}
{%- if existsIn(rng, "nizbel") -%}
[Backup](/speedruns/ct/rng-manip-100-simple/backups/nizbel/)

{% include "nizbel_battle" %}

{%- endif -%}
{%- if existsIn(rng, "flea") -%}
{% include "flea_battle" %}

{%- endif -%}
{%- if existsIn(rng, "magus") -%}
[Backup](/speedruns/ct/rng-manip-100-simple/backups/magus/)

{% include "magus_battle" %}

{%- endif -%}
{%- if existsIn(rng, "nizbel2") -%}
{% include "nizbel2_battle" %}

**Black Tyranno**

[Backup](/speedruns/ct/rng-manip-100-simple/backups/blacktyranno/)

{%- endif -%}
{%- if existsIn(rng, "mudimp") -%}
## Load Seed: 1919209325 (TimeZones.UTC, 26, 10, 2030, 01, 42, 05)

{% include "mudimp_battle" %}

{%- endif -%}
{%- if existsIn(rng, "golemtwins") -%}
[Backup](/speedruns/ct/rng-manip-100-simple/backups/golemtwins/)

**Golem Twins**

{%- endif -%}
{%- if existsIn(rng, "ghosts") -%}
**Ghosts**

**Ozzie's Fort**

After guillotine room

Party (Magus / Ayla / Marle)

* Frog <-> Magus

* Magus (**Doom Scythe** / **Gloom Helm** / **Gloom Cape** / **Berserker Ring**)
    * Doom Scythe
    * Gloom Helm
    * Gloom Cape
    * Berserker Ring

**Ozzie, Flea, and Slash**
{{<battlebox>}}Auto-battle when Slash kills Ayla{{</battlebox>}}

**REMOVE BERSERKER FROM MAGUS!!!**

Party (Magus / Lucca / Robo)

* Lucca <-> Ayla
* Marle <-> Robo

* Magus (Doom Scyth / Gloom Helm / Gloom Vest / **Guardian Bangle**)
    * Guardian Bangle

{%- endif -%}{%- if existsIn(rng, "rusttyranno") -%}
**Rust Tyranno**

[Backup](/speedruns/ct/rng-manip-100-simple/backups/rusttyranno/)

{%- endif -%}
{%- if existsIn(rng, "sonofsun") -%}
**Son of Sun**

[Backup](/speedruns/ct/rng-manip-100-simple/backups/sonofsun/)
{%- endif -%}{%- if existsIn(rng, "yakraxiii") -%}

**Yakra XIII**
{%- endif -%}