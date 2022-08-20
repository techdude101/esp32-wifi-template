#ifndef _HTML_FORM
#define _HTML_FORM

const char* htmlForm = "<!DOCTYPE html><html lang=en><meta charset=UTF-8><meta content=\"IE=edge\" http-equiv=X-UA-Compatible><meta content=\"width=device-width,initial-scale=1\" name=viewport><title>Minimal Web Form</title><style>:root{--color-primary:#F8F9FA;--color-secondary:#212529;--color-text-primary:#000;--color-text-secondary:#FFF}body{margin:0 auto;font-family:'Courier New',Courier,monospace;color:var(--color-text-primary);background-color:var(--color-primary)}h1{text-align:center}form{line-height:1.5rem;display:flex;flex-direction:column;width:20ch;max-width:80%;margin:0 auto}input,label{margin-bottom:.5rem;padding:.5rem .3rem}input[type=submit]{border:none;color:var(--color-text-secondary);background-color:var(--color-secondary);margin-top:.5rem;letter-spacing:.15ch}</style><h1>WiFi Settings</h1><form action=\"config\" method=POST><strong><label for=input-name>Network Name</label></strong> <input id=input-name name=input-name> <strong><label for=input-password>Password</label></strong> <input id=input-password name=input-password> <input type=submit value=Submit></form>";

#endif
