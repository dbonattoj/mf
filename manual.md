---
layout: page
title: Manual
---

{% for pg in site.manual %}
- [{{ pg.title }}]({{ pg.url | prepend: site.baseurl }})
{% endfor %}
