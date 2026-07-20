#include "common_css.h"


const char COMMON_CSS[] =

"*{"
"margin:0;"
"padding:0;"
"box-sizing:border-box;"
"}"


"body{"
"font-family:'Segoe UI',Arial,sans-serif;"
"height:100vh;"
"background:linear-gradient(135deg,#0f2027,#203a43,#2c5364);"
"color:white;"
"display:flex;"
"justify-content:center;"
"align-items:center;"
"overflow:hidden;"
"}"


".card{"
"width:420px;"
"padding:35px;"
"border-radius:20px;"
"background:rgba(255,155,255,0.08);"
"backdrop-filter:blur(18px);"
"border:1px solid rgba(255,255,255,0.15);"
"box-shadow:0 15px 40px rgba(0,0,0,0.45);"
"text-align:center;"
"z-index:2;"
"}"



"h1{"
"font-size:34px;"
"color:#58a6ff;"
"margin-bottom:20px;"
"}"


"h2{"
"font-weight:300;"
"color:#d0d8e2;"
"margin-bottom:20px;"
"}"


".info{"
"font-size:16px;"
"line-height:2;"
"color:#d8dee9;"
"}"


".status{"
"display:inline-block;"
"padding:10px 25px;"
"margin:15px;"
"border-radius:30px;"
"background:#00c853;"
"font-weight:bold;"
"color:white;"
"}"

/* Service / Status boxes */

".service-item{"
"background:rgba(255,255,255,0.08);"
"padding:12px;"
"margin:12px 0;"
"border-radius:12px;"
"display:flex;"
"justify-content:space-between;"
"align-items:center;"
"}"


".online{"
"color:#00ff88;"
"font-weight:bold;"
"}"


".ready{"
"color:#58a6ff;"
"font-weight:bold;"
"}"



/* Bottom Navigation */

".bottom-nav{"
"position:fixed;"
"bottom:15px;"
"left:50%;"
"transform:translateX(-50%);"

"width:95%;"
"max-width:550px;"
"height:75px;"

"background:rgba(22,27,34,0.85);"

"backdrop-filter:blur(15px);"

"border-radius:25px;"

"display:flex;"
"justify-content:space-around;"
"align-items:center;"

"box-shadow:0 10px 30px rgba(0,0,0,.5);"

"border:1px solid rgba(255,255,255,.15);"

"z-index:10;"
"}"



".bottom-nav a{"
"color:#b8c1cc;"
"text-decoration:none;"
"font-size:12px;"
"display:flex;"
"flex-direction:column;"
"align-items:center;"
"transition:.3s;"
"}"


".bottom-nav span{"
"font-size:26px;"
"}"


".bottom-nav label{"
"margin-top:4px;"
"}"


".bottom-nav a:hover{"
"color:#58a6ff;"
"transform:translateY(-8px);"
"}"



"@media(max-width:500px){"

".card{"
"width:90%;"
"padding:25px;"
"}"


".bottom-nav{"
"height:70px;"
"}"


".bottom-nav span{"
"font-size:22px;"
"}"

"}";
