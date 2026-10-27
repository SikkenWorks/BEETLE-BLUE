from playwright.sync_api import sync_playwright
from pathlib import Path
base=Path(__file__).resolve().parents[2]/'web'
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox','--disable-web-security','--autoplay-policy=no-user-gesture-required'])
 for file in ['simulator.html','BEETLE-BLUE-Project-Manager.html','WebInstaller.html','BEETLE-BLUE-Manager.html']:
  page=browser.new_page(viewport={'width':1200,'height':950})
  errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  try:
   page.set_content((base/file).read_text(),wait_until='domcontentloaded',timeout=20000)
   page.wait_for_timeout(600)
   print(file, 'errors',errors[:3],'title',page.title())
   if file=='simulator.html':
    page.locator('#lane').select_option('2')
    page.locator('#seqHold').click();page.locator('#selNext').click()
    # internal project is top-level lexical, accessible to eval from page
    val=page.evaluate('project.laneKits[2]')
    print('SEQ+SELECT CH3 kit',val)
    assert val==1
   if file=='BEETLE-BLUE-Manager.html':
    page.locator('#tab-projectmanager').click()
    page.wait_for_timeout(400)
    print('manager tab visible',page.locator('#view-projectmanager').is_visible())
  finally:page.close()
 browser.close()
