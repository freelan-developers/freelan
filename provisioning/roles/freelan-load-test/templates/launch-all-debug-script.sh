#!/bin/bash

{% for node in freelan_nodes %}
{{ scripts_root }}/detached-debug-{{ node.name }}.sh
{% endfor %}
