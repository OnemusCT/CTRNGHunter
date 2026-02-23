<!-- 
The format of these templates is catered towards onemus.net.
If you aren't me I'm sorry.
-->
**Load Seed: {$ seed $}**

{%- if existsIn(rng, "yakra") -%}
{% include "newgame_full" %}

{% include "yakra_full" %}
{%- endif -%}
{%- if existsIn(rng, "dragontank") -%}{% include "dragontank_full" -%}

{% include "guardian_full" %}

{% include "rseries_full" %}

{% include "heckran_full" %}
{%- endif -%}
{%- if existsIn(rng, "zombor") -%}
{% include "zombor_full" %}

{%- endif -%}
{%- if existsIn(rng, "masamune") -%}
{% include "masamune_full" %}

{%- endif -%}
{%- if existsIn(rng, "nizbel") -%}
{% include "nizbel_full" %}

{%- endif -%}
{%- if existsIn(rng, "flea") -%}
{% include "flea_full" %}

{%- endif -%}
{%- if existsIn(rng, "slash") -%}
{% include "slash_full" %}

{%- endif -%}
{%- if existsIn(rng, "magus") -%}
{% include "magus_full" %}

{%- endif -%}
{%- if existsIn(rng, "nizbel2") -%}
{% include "nizbel2_full" %}

{%- endif -%}
{%- if existsIn(rng, "blacktyranno") -%}
{% include "blacktyranno_full" %}

{%- endif -%}
{%- if existsIn(rng, "mudimp") -%}
{% include "mudimp_full" %}

{% include "gigagaia_full" %}

{% include "golemtwins_full" %}

{% include "dalton_full" %}
{%- endif -%}
{%- if existsIn(rng, "rusttyranno") -%}{% include "rusttyranno_full" %}
{%- endif -%}
{%- if existsIn(rng, "sonofsun") -%}{% include "sonofsun_full" %}

{% include "yakraxiii_full" %}

{% include "motherbrain_full" %}

{% include "melphyx_full" %}

{% include "gigamutant_full" %}

{% include "lavosspawn_full" %}

{% include "queenzeal_full" %}

{% include "lavosshell_full" %}

{% include "lavoscore_full" %}
{%- endif -%}
