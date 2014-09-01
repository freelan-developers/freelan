#!/bin/bash

{% for node in freelan_nodes %}
{{ scripts_root }}/detached-{{ node.name }}.sh
{% endfor %}
