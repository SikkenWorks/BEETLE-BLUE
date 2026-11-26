// SPDX-License-Identifier: GPL-3.0-only
// Offline JavaScript regression for both pinned Felucca-family loader packages.
const fs=require('fs'),vm=require('vm'),path=require('path'),crypto=require('crypto');
const r=path.resolve(__dirname,'..');
const html=fs.readFileSync(path.join(r,'WebInstaller.html'),'utf8');
const scripts=[...html.matchAll(/<script[^>]*>([\s\S]*?)<\/script>/g)].map(m=>m[1]);
const main=scripts.find(s=>s.includes('const AUDITED_LOADERS ='));
if(!main)throw Error('WebInstaller dual-loader guard missing');
const ctx={window:{},document:{addEventListener(){},getElementById(){return {addEventListener(){},style:{},hidden:false,textContent:'',value:''}}},navigator:{},console,TextDecoder,TextEncoder,crypto:crypto.webcrypto,Uint8Array,ArrayBuffer,DataView,performance};
ctx.window.crypto=ctx.crypto;
vm.runInNewContext(main,ctx);
const api=ctx.window.ETCHOta;
if(!api)throw Error('Installer OTA audit API missing');
(async()=>{
 for(const name of ['BEETLE-BLUE-V1.3.1.fwsc','reference/v1.3-reference.fwsc']){
  const raw=new Uint8Array(fs.readFileSync(path.join(r,name)));
  const layout=api.verifyFwscLayout(raw);
  if(![6930,7312].includes(layout.ota.len))throw Error('Wrong loader '+name);
  const result=await api.auditGenericPackage(raw);
  if(result.product!=='FM-1_2224')throw Error('Wrong identity '+name);
  console.log('PASS JS loader+APP audit:',name,layout.ota.len,'bytes');
 }
})().catch(e=>{console.error(e);process.exitCode=1});
