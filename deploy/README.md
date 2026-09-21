# Deploy notes — Docsify notes + AGVS HTML courses

## What needs Nginx config?

**No special `location` for `6_AGVSinAction`.**  
Courses are plain static HTML. If Nginx `root` is the repo root, these URLs work immediately after deploy:

| URL | Page |
|-----|------|
| `/` or `/#/README` | Docsify notes home |
| `/6_AGVSinAction/` or `/6_AGVSinAction/index.html` | Course catalog |
| `/6_AGVSinAction/09HardwareSelectionInterface/index.html` | One course TOC |
| `/6_AGVSinAction/09HardwareSelectionInterface/00CourseIntro.html` | One chapter |

## Recommended Nginx

See [`nginx-personal-blog.conf`](./nginx-personal-blog.conf).

Key points:

1. `root` = repository root (contains both `index.html` and `6_AGVSinAction/`)
2. `try_files $uri $uri/ /index.html;` — **`$uri` first**, so real HTML files are served before Docsify fallback
3. `charset utf-8;` — Chinese file names / content
4. `index index.html;` — so `/6_AGVSinAction/` resolves without typing `index.html`

## Redeploy checklist

```bash
# on server
cd /var/www/PersonalBlogWeb   # or your actual path
git pull

# first-time only: install sample nginx site
sudo cp deploy/nginx-personal-blog.conf /etc/nginx/sites-available/personal-blog
# edit root/server_name if needed
sudo ln -sf /etc/nginx/sites-available/personal-blog /etc/nginx/sites-enabled/
sudo nginx -t && sudo systemctl reload nginx
```

## Docsify links

In Markdown, HTML course links use absolute paths and Docsify `:ignore` so the SPA does not intercept them:

```markdown
[AGVS 实战课程](/6_AGVSinAction/index.html ':ignore')
```

Navbar uses `/6_AGVSinAction/index.html` (full page navigation, leaves Docsify hash route).
