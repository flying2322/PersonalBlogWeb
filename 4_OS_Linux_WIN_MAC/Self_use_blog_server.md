1. [CodeSheep knowledeg server](https://www.bilibili.com/video/BV1eu411m797/?spm_id_from=333.999.0.0&vd_source=a61087f39589ca585073e8548bdf0de5)
- hixo.org
- VuePress
- docsify

```shell
npm -v
npm install -g docsify-cli
docsify -v

# go to the Folder whether your website want to store 
docsify init
```
Then check whether the docsify was installed successfully.
- vue.css
- buble.css
- dark.css
- pure.css
- dolphin.css

---

## Production deploy (Docsify + AGVS HTML)

See **[deploy/README.md](../deploy/README.md)** and sample Nginx config **[deploy/nginx-personal-blog.conf](../deploy/nginx-personal-blog.conf)**.

`6_AGVSinAction/` is static HTML — no extra Nginx location needed if `root` points at the repo.


