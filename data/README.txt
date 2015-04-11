CyberDragon Browser is an privacy enhanced browser that protects 
your surfing habits.

Still in early beta but the following features have been implemented:

- Tracker blocker. It will block over 6000 various trackers from 
  following your daily surfing habits. Later version will allow 
  user defined tracker blocker rules that can be exported/imported
  and also automatic master tracker blocker file updater to always
  keep your surfing data safe from bad guys.

- Very strict, zero-tolerance cookie control. By default, only 
  session cookies that have Secure and HttpOnly cookie attributes 
  set are allowed. Also, all 3rd party cookies are blocked by
  default. These settings can be overrided with custom domain
  specific cookie rules. 

  So in other words, the default cookie policy for CyberDragon
  is to *deny* all cookies except those that either meet the 
  strict criteria of global cookie settings or those cookies 
  that are explicitly allowed by adding them to custom cookie 
  rules list.

- Proxy fetcher. Will allow you to fetch high-anonymous proxies that
  you can use to hide you IP-address with just 2-clicks 
  (one click to fetch proxies and second click to select proxy from
  the list)


For donations:
http://www.binarytouch.com/windows.htm

For feedback:
http://www.binarytouch.com/contact.php
or
cyberdragonbrowser@gmail.com

Blog:
http://sourceforge.net/p/cyberdragonbrowser/blog/?source=navbar

1.6.6:
---------------------------------------
It's now fully Open Sourced!!!
Big thanks to all the nice people for donations!

Im going to semi-retire from developing (just don't have time anymore to work with it full time)
But this does not mean that development has stopped. I still accept patches and fixes.
Just send them to above address (in "diff -Naur" format please) and I will take a look at them. 


1.6.5:
---------------------------------------
It has been a while but finally I had time to fix/add things in CyberDragon.

Minor improvements:

-	Added option to use web inspector. You can access this by right clicking 
	the page you are interested and choosing "Inspect" from context menu.
	That way you can inspect page HTML source and DOM tree.

-	Added WebGL enable/disable option. As the CyberDragon does not
	currently support *real* Hardware Acceleration (that's why 
	"Accelerated Composing" is still disabled from General tab)
	I suggest that you keep it disabled. 

	If you enable it you will enable slow software acceleration 
	and even worse, make yourself more vulnerable to browser fingerprinting. 
	So enable it only if for some reason you *really* need WebGL.

-	Added "Disable Google Search Autocomplete" option to General tab.

-	Added "Prevent E-Tags tracking" option to General tab.	

-	Changed the default home page from https://startpage.com to
	https://startpage.com/do/mypage.pl?prf=f20d137624e7f67b0ad1d107f0796688.

	Because CyberDragon does not store cookies currently to disk the
	only other way to permanently store startpage settings is to 
	use custom URL like above (you can check the settings by clicking
	the "Settings" link at the top of startpage).

	In short, I choosed the following startpage settings as default:
	- More results per page (100)
	- Enable Geogrpahical maps (allows doing Google static maps lookups
	  while still retaining your privacy)
	- Do not use family filter for pictures.
	- Do not use video filter.
	- Anonymous images/video search 
	(this uses startpage proxy, ixquick-proxy.com)

	If you don't like the above settings you can always generate your own
	custom URL by clicking "Generate URL" from startpage settings page 
	and set it as your new home page.
	
-	Added ixquick-proxy.com to "Disable MCB for following URLs" list.

-	Added browser history drop-down menu.

Major improvements:

-	Fixed YouTube crash when closing tab.

-	Improved HTTP Referer controls from General tab.
	Previously you could only choose between sending HTTP Referer or
	completely removing it.
	Now you have two additional choices: Send the same URL as the currently
	open page as HTTP referer or send your own custom HTTP referer value.

-	Added Logger tab (key shortcut Alt + L). This is for debugging 
	purposes only and to find new trackers.
	Note: Logging slows things and doubly so if you choose
	to log to file. Use only when needed.

-	Added Plugins tab (key shortcut Alt + U). There you can choose
	individual plugins that you want to enable/disable.

-	Added Server Info tab (key shortcut Alt + S).
	When enabled, you can see little details of the server you are
	in contact, like it's IP addresse(es), country, city etc...

	Also, if you want, you can check it's location from Google maps.
	Note that this functionality needs MaxMinds GeoLiteCity.dat (free version)
	or GeoCity.dat (paid version) to work. 
	Paid version is more accurate and up-to-date.

-	Updated included Qt Framework library from 5.2.1 to 5.4.0.

-	SECURITY UPDATE: Updated included OpenSSL library from 1.0.1g to 1.0.1j.

-	Disabled all the ciphers using SSLv3 protocol because poodle attack.
	In next version I will completely remove all the ciphers using SSLv3.

-	Default Blocked trackers increased from 6,000 to 30,000.
	Changed the default tracker blocker rule list filters.txt to 
	filters_optional.txt.

	I feel that the filters_optional.txt list is now good enough to start using it.

	So the good news are that now CyberDragon will block even more trackers.
	However, it might be a little bit slower than old list. Old list is still
	available if you want it.


Then some news: Qt has freezed QtWebKit development as of 5.4.0. 
That means that new features are not going to be added to QtWebKit. 
As for bug fixes to QtWebKit, I have not yet received any answer from Qt. 

So what does this all mean for CyberDragon? It means that because CyberDragon
uses QtWebKit as it's page rendering engine (the thing that makes those web pages work) 
that at some point I have to stop using it and switch either to Qt´s new QtWebEngine 
or maybe completely to another framework. 

Needless to say that all will be a huge work for me and Im not sure 
if I can make it without support.

Another news: I have been thinking long and hard of the future of CyberDragon codebase.

To keep it alive I would like to open source it so that everybody can contribute. 

However, I need a new laptop now. Badly. 
And my home server is starting to get old too. So as a solution, if I manage to
get 8000 euros as donation then I will be more than happy to open source it with
GPLv2 license. 

So if each CyberDragon downloader could donate, for example, 
about 5 euros (about 6 USD at the moment), then I would be very gratefull.

There are several ways to donate, listed here in order of preference:

1.	Direct transfer to bank account throught online wire transfer.
	In addition to banks, there are also other financial service companies like
	Western Union (https://www.westernunion.com/us/en/direct-to-bank-details.html)
	and others that do money transfer.

	No matter who the financial service provider is, you will usually need the
	following information: 
	receiver name, bank name, BIC/SWIFT and IBAN number.
	Also they will usually take some small fee.

	Here's my bank details if you like to use direct transfer to bank account:

	Receiver name:	STEFAN FRÖBERG 
	Bank name:	TURUN SEUDUN OSUUSPANKKI, HÄMEENKADUN KONTTORI
	IBAN:		FI69 5711 1320 1093 14
	BIC/SWIFT:	OKOYFIHH

	Or if instead of Euros you would like to directly send USD then you can use:

	Receiver name:	STEFAN FRÖBERG 
	Bank name:	TURUN SEUDUN OSUUSPANKKI, MAARIANKADUN KONTTORI
	IBAN:		FI56 5710 0431 1031 42
	BIC/SWIFT:	OKOYFIHH

	In both cases receiving country is Finland.
	If any message field needs to be filled (like usually is the case with banks)
	then just fill it with: "CyberDragon donation"

2.	Bitcoin.
	From the http://www.binarytouch.com/windows.htm on the right side there is my 
	bitcoin account address that you can use for donation. Also QR Code is provided
	for convenience. 

	If the page is unreachable for some reason then you have to manually enter the
	following bitcoin address to do donation:
	17uPzWN5YyzDnzcGBSDAotbKUB8tk8uCEK

3.	PayPal.
	Also from http://www.binarytouch.com/windows.htm you can do a donation throught
	PayPal. However, this is the least preferable solution because it currently
	has 2500 euros yearly limit and I still haven't lifted it.

So at 5 euros/download and at the current download rate of about 200 per week, 
it would take just two months to reach the target and open sourcing to happen.

Lastly: Linux version of CyberDragon 1.6.5 will be available after December.

Surf Safe and have a Merry Christmas!


1.6.4:
---------------------------------------

Minor improvements:

-	Added URL percent decoding.

	For more details, please see:
	http://sourceforge.net/p/cyberdragonbrowser/blog/2014/03/patch-for-cyberdragon-163---https-enforcing---google-tracking-redirect-links-removal-feature/

-	Add "Save as PDF" button (can be found from top left).
	This will let you "print" the currently open
	web page as an PDF file.

	For more details, please see:
	http://sourceforge.net/p/cyberdragonbrowser/blog/2014/03/patch---cyberdragon-163---add-save-as-pdf-functionality/

-	Added Content-Dispostion header support.
	For more details, please see:
	http://sourceforge.net/p/cyberdragonbrowser/blog/2014/03/patch-cyberdragon-163---add-content-disposition-support/

Major improvements:

-	Updated included Qt Framework library from 5.2.0 to 5.2.1.

-	SECURITY UPDATE: Updated included OpenSSL library from 1.0.1e to 1.0.1g.
	Please see: https://www.openssl.org/news/secadv_20140407.txt


1.6.3:
---------------------------------------
Ladies and Gentlement! Let me introduce you with the latest
version of CyberDragon!

Without further delay here are the changes:

Major improvements:

-	Fixed image showing bug. For some users
	images did not show when starting the browser.
	This should now be fixed.

-	HTTPS Enforcing. 
	This feature is similar to EFF HTTPS Everywhere 
	Firefox extension.

	However, where EFF's HTTPS Everywhere does use a
	lots of regular expression rules to handle most of
	it's https handling CyberDragon tries
	to solve the same thing with as little as
	possible of those rules 
	(there is currently only one rule and that's for Googles case)

	HTTPS Enforcing actually packs three features with
	the price of one. One general and two Google specific.

	Firstly: 
	--------------------------------------------------------
	The general feature of HTTPS Enforcing, like the name 
	suggest, tries to use HTTPS protocol in as many sites as possible.

	So everytime you type some www-address (without the http:// part)
	it will try to load the https:// version, if available/working.

	Also, everytime you click some http:// link it will first try
	to load https:// version and again, if none is available/working
	it will fallback to http:// version.

	Sadly, not all the sites offer alternate https:// version
	of their content. And even those that do offer the https://
	version of their content are often broken/buggy. 
	In these cases there really is no other way than use http:// 
	version instead.

	Secondly: 
	--------------------------------------------------------
	Activating HTTPS Enforcing will also disable JavaScript 
	for Google Search 
	(for both www.google.* and encrypted.google.* domains)

	The reason is that Google loads some pretty crazy
	JavaScript stuff in background that does not play nicely
	with HTTPS Encorcing! 
	*Especially* when you have User-Agent spoofing enabled 
	and you have chosen to impersonate your browser as 
	"Chrome" or "Firefox" 
	(Google does some browser sniffing in addition to it's 
	JavaScript mumbo jumbo)	
	
	So you will loose Google Autocompletion (booo-hooo!) but in
	return you will get faster and safer connection to google
	search that uses less your bandwidth (yay!).

	Thirdly: 
	--------------------------------------------------------
	And lastly there is this thing called "Google Tracking Redirect links". 
	When you search something (like let's say "Knight wikipedia")
	from Google and then move your mouse over the link 
	you will see something like this on CyberDragon statusbar at the bottom:

	https://www.google.fi/url?q=http://en.wikipedia.org/wiki/Knight
	&sa=U&ei=tLckU96_CcuJhQf7nYG4Bw&ved=0CCAQFjAA&
	usg=AFQjCNFoh5hRriPh-IyM5PekEfve73V5Ww 

	That's Google doing it's tracking when (if) you click that search
	result link. And even disabling JavaScript won't help against that!

	There are two ways to change that horrible thing to correct,
	http://en.wikipedia.org/wiki/Knight version:

	1) Use the browser's User-Agent spoofing and lie that you are 
	   using either Chrome or Firefox. If you do that then "magically"
	   Google won't show you anymore tracking redirect links in it's
	   search results but the normal correct links. 

	   Actually, when you remember the fact that Chrome is owned &
	   developed by Google and that Firefox itself get's 85% of it's
	   annual income from Google then maybe this is not surprising at all ...

http://techcrunch.com/2012/11/15/mozilla-releases-annual-report-for-2011-revenue-up-33-to-163m-majority-from-google/

	2) Enable HTTPS Enforcing and let it dynamically rewrite all 
	   those filthy search result links to their normal, correct, 
	   non-tracking form.

	So there! You get three features with the price of one :-)

	However, keep in mind that this is still experimental feature
	and it needs a lot of testing (that's why I didn't enable it by default)

	For those brawe souls who want to try it right now head to 
	"Encryption" tab, check the "Try to use HTTPS whenever possible" 
	option and do some surfing/searching on Google and see how many 
	http:// links are automatically loaded to https:// version 
	instead when clicked over (wikipedia.org at least does).

Last note: I am beginning a practise of including credits.txt file 
with CyberDragon. 
That file will include the names and (if wanted) emails of the people who have 
some way or other helped fixing problems. 

Only names with written permission are included. If your name is not on that list 
(I might have forgotted contacting you or not gotted any response) and you want 
it included then please contact me.

Surf Safe!
	
1.6.2:
---------------------------------------

Major improvements:

-	Fix crash during startup.

	This version should fix the crash when trying to startup CyberDragon.
	Big thanks go to both Steve (for reporting this issue with Windows XP)
	and to Vijay (for reporting this issue with Windows 7 64-bit).

	Those of you who still have both 1.6 and 1.6.1 zip files somewhere 
	don't necessarily need to waste your bandwidth for downloading this
	version.

	Just follow the steps from:

http://sourceforge.net/p/cyberdragonbrowser/blog/2014/02/important-note-for-windows-xp-users/

	Others can just skip 1.6.1 and go directly to 1.6.2


	And now here's the story behind this crash:

	About two weeks ago my Windows 7 64-bit development laptop 
	crashed and I had to reinstall everything, windows & development 
	tools, from scratch. 
	(luckily I had made backups of CyberDragon sources!).

	Now, if my memory servers me well (and it rarely does) the old 
	version of CyberDragon, 1.6, was compiled against Qt 5.2. 

	And I was pretty sure that I used that Qt version with 1.6.1 
	too when compiling after crash.

	But it now seems that there were three files under "platforms" 
	subfolder that did not belong to Qt 5.2 but instead to 5.2.1 
	that I had accidentally installed after crash.

	So now platforms folder should contain the right files and 
	everything should be fine.
	

	Linux version was done with different laptop so that's 
	why there is no update for CyberDragon Linux version.

	Sorry about all the hassle.
	
	(but I still don't understand why my local copy of 1.6.1 
	has not been crashing during startup...)


1.6.1:
---------------------------------------

Boy, I have been very busy lately. 
And not just coding stuff but other stuff too.
But now I have finally managed to put final pieces to both 
Windows & Linux versions of CyberDragon Browser v1.6.1.

Here's the stuff so far:

Minor improvements:

-	It will now save "Clear Cookie list on page load" 
	setting on exit.

-	Also "Supported Ciphers & their order of preference" 
	settings are now saved as well on exit.

- 	Fixed crash that happened if you had several tabs open, 
	closed the latest one and then pushed 
	"Clear Cache Now!" button.

-	Added show/hide settings button 
	(it's that blue arrow on top right).

- 	Added button to copy URLs from "Blocked Content URLs" 
	view to "Disable Mixed content for following URLs" 
	view to Encryption tab.

Major improvements:

- 	Linux version. It's just a self-standing tar package now with 
	all the necessary libs included. 
	I have tested it with both Fedora 20 and Ubuntu 12.04.4 LiveCDs
	with no extra stuff installed.
	
	After extracting it just cd to it's folder and 
	type ./CyberDragon from shell to start it. 
	Had no time to make RPM, sorry. 

	However, if you are proficient of making rpm SPEC
	file then send it to me for checking it. 
	You can send it to cyberdragonbrowser@gmail.com

	For more information how to use CyberDragon linux version
	please see Appendix C. Linux on CyberDragonManualDraft.pdf

-	Added FTP server browsing support.

	Previously, you could download stuff that had
	ftp:// in front of it's URL just fine like:

	ftp://ftp.mars.org/pub/mpeg/libmad-0.15.1b.tar.gz

	But you could not browse and see the contents of any
	public FTP server like ftp://ftp.mozilla.org/pub

	Now you can do that too.

- 	Added certificate information to Encryption tab.

- 	Fixed the Cipher information that is shown for current
	connection on Encryption tab. 
	Now it should show correct information.

	Important Note: If you change any cipher order or
	enable/disable them, the changes won't come to effect
	untill you tear down the curremt https connection by closing 
	the currently open tab and reopening it.

	Important Note 2: During testing Google https encryption
	I found the following. When visiting https://www.google.com
	it redirected me to my local language version 
	https://www.google.fi. What I found there was that it used
	weak encryption cipher RC4 (that NSA has supposedly tampered with)!

	So as a safety mechanism I decided to move all four RC4 ciphers
	to very bottom of the cipher order list and disable them.

	That will force https://www.google.com to use stronger
	encryption cipher.

	For https://encrypted.google.com this was not necessary and
	it used AES cipher immediately.

- 	Added user corfirmation dialog when trying to visit web site 
	with self-signed SSL certificate.
	Before this, all the self-signed SSL connections were 
	blocked by default. Now it ask if you are sure you 
	want to continue.

- 	Added User-Agent spoofing possibility to General tab.
	Please see General chapter on CyberDragonManualDraft.pdf

- 	Added HTTP referer removing possibility to
	General tab.
	Please see General chapter on CyberDragonManualDraft.pdf

- 	Added Google PREF cookie spoofing possibility to 
	Cookie tab. This should not normally be needed as 
	CyberDragon will by default block all PREF cookies sent
	by Google. 

	Consider this as a experimental feature and
	use it *only* if you use some google service that needs
	PREF cookie enabled 
	(so far I could only find SafeSearch using it).

	It can be set to some particular value or set to
	randomly generate it each time you visit google pages.

	For more information please see:
	
http://repository.cmu.edu/cgi/viewcontent.cgi?article=1058&context=jpc

	And here's the original story how NSA is using PREF
	cookies to locate it's hacking targets:

http://www.washingtonpost.com/blogs/the-switch/wp/2013/12/10/nsa-uses-google-cookies-to-pinpoint-targets-for-hacking/


- 	Added few more proxy sources to proxy fetcher. On a good day, 
	you could get an average of about 140 proxies when clicking 
	"Get Proxies" button.




1.6:
---------------------------------------
	Originally I wanted to do just minor update but then I came to
	second thoughts and decided to skip the 1.5.5 version and jump
	directly to 1.6.

	So here are the new & fixed stuff so far:

Minor improvements:

-	Fixed bug that prevented blocked custom cookie rule to be shown on
	Cookie List view

-	Fixed sorting bug on Cookie List view. Now they are correctly sorted
	so that the last arriwed cookie is at the bottom of the Cookie List.

-	Fixed editing bug of custom cookie view if cookie action had selected
	as Blocked

-	Now when you click "move cookie to custom cookie view" button it will
	automatically replace any dot (.) character with \. for domain names.

-	Now when you click "Add exception to mixed content blocker" button it 
	will automatically replace any dot (.) character with \. for 
	domain names.
	It will also by default add the currently open tab URL to it.

-	Added optional filters_optional.txt file that has over 30 000 tracker
	blocker rules. You can change your current tracker blocker list by
	pushing "Browse ..." button from Tracker blocker tab.
	However, keep in mind that this list has not been optimized, it will
	contain redundant entries and it has not completely checked as yet.
	Also, it will make your surfing slower than the default filters.txt
	list.

Major improvements:

-	Upgraded Qt 5.1.0 to 5.2.0. Hopefully this does not introduce
	new bugs.
	At least it fixed the printer bug where the first printed page was
	looking funny.

-	Fixed Check/Stop Check proxy button. It had a bug that prevented you
	to do proxy checking more than once if you allowed the first check
	to complete (that is you, did not stop it between checks)

-	Random proxy hopping works again!

-	Added blacklist proxy option. This was needed because I found out
	that proxy checker anonymous checker was not 100% accurate.
	Out of 84 manually tested proxies 3 proxies were actually
	non-anonymous even tought the proxy checker had marked them
	as anonymous!
	So there is a bug in anonymous checker and I will try to fix it after
	holidays. In the meantime, you should manually recheck and corfirm
	that the proxies that have anonymous marked as "Yes" will indeed be
	anonymous and if not, blacklist them.

-	Fixed MCB (Mixed Content Blocker). No more is MCB blocking you if
	you are being redirected from https:// page/site to 
	http:// page/site.
	Also removed redundant entries from "Disable mixed content for
	following URLs".
	I left few entries there but made them disabled, just in case if
	they are needed.

-	Added Supported ciphers view to Encrypted tab so that you can decide
	which ciphers and in what order, are being tried when making
	HTTPS connection.

-	Added view to Encrypted tab that will show the cipher that was used
	for that particular https:// page.

-	Fixed documentation for the regular expression stuff at
	Tracker blocker chapter, rewrited Encryption chapter and added
	Proxy chapter and how to use Tor with CyberDragon chapter.

-	Added file information to downloads tab and it will now show you all
	the little details like name,path,size, mimetype and SHA1 checksum
	when you click the finished filename from downloads view.

Merry Christmas!

1.5.4:
---------------------------------------
	Finally managed to get integrate all the protections with
	tabs. Had to rewrite at least 75% of the code and had to ditch
	the Open Window policty for scripts (it was clunky anyway
	and must rethink later) thing and also the option to 
	"Open Link in New Window", which, I think is a small loss
	because people use tabs mostly anyway.

	So here it is, without further delay, the changes made so far:

Minor improvements:

-	Fixed the HTTP Pipeling handling for POST request. Now
	The default is to use HTTP Pipeling for everything else
	except for Proxies and POST request.

-	Cleaned alot of clunky stupid code (that made sense at the
	time I writed it) and hopefully made CyberDragon more
	faster and less bugs.

-	Added Browse button to Cache tab so that you can much more
	easily select your cache folder.

-	Rechecked the proxies.txt file and throw out duplicates,
	non-working proxies and non-anonymous proxies. This
	left a file called anonymous_proxies.txt that contains	
	about 30 proxies for your use.

-	Fixed that annoying flicker when switching from NOT CLEAN
	to CLEAN state (and vise versa) at tracker counter.

-	Added What's this button that with clicking it you will get more
	information of some of the elements. Currently I only had
	time to add information to General Settings tab settings.

-	Added Alt + D key shortcut for Downloads tab for easier access.

Major Improvements:

-	Rewrited the proxy handling code for fetching/importing/checking.
	Now it is possible to fetch proxies without checking them,
	import proxies without checking them or both, import AND fetching
	more proxies without checking them. The reason for this is that
	you might already have a good proxy list handy so there is no
	point of checking it over and over again each time.
	Now, when you want to check the proxies just push "Check Proxies"
	button. Also, now you can remove proxies from list (just right click
	and select remove proxy from pop up context menu) and export only
	proxies you have selected instead of exporting them all.

-	Added support for SOCK5 proxies (namely, Tor). The default is
	still to use HTTP(S) proxies. Note: The proxylist checker does 
	not currently check if the proxies are http(s) or socks5.

-	Added lot's of notifications for proxy events, tracker events and
	most importantly for mixed content blocked event.

-	Added blocked mixed content view to Encryption tab so that you can
	actually see in realtime what were blocked.

-	Major overfaul of cookie system. So major that it takes too much
	of explaining it here. Please read the included 
	CyberDragonManualDraft.pdf file for more information.

-	You can now finally add your own tracker rules, change tracker rules,
	remove tracker rules, export tracker rules and import tracker rules.

-	Started finally writing manual for CyberDragon. Just at draft stage
	now. See included CyberDragonManualDraft.pdf

-	And lastly, the most major improvement of all, tabs.
	Yes you can finally open content (image or link) into new tab
	with right clicking it and selecting "Open Link in New Tab".
	If you want to open completely new, empty tab press Ctrl + T.
	If you want to close current tab press it's close button or 
	alternatively key shortcut Ctrl + W.
	Tabs are fully moveable.
	If you accidentally close last tab nothing bad happens
	but then you have to open new tab with Ctrl + T to continue.

So there. Next version 1.5.5 will be mostly just bug fixes and
making documentation better. And maybe adding even more stuff to Proxy tab.
It's already quite crowed with my small 15.6" screen so I had to put stuff
to scrollarea.

Read the manual and Surf Safe!

---------------------------------------
1.5.3:	Christmas is coming this year early and Santa brings you
        new toys. So without further delay, here is CD 1.5.3!

Minor improvements:

- You can now finally set your own home page so that you don't
  have to stare https://startpage.com each time you start CD.

  Just enter what you like to "Home page" field at "General"
  tab or alternatively, push "Set currently open page" to set
  page you are viewing as your home page.

- Added logo (yay!).Unfortunately you have to watch it with
  magnifying glass. Im not much of an artist.

- Added "Open new window action policy for scripts" to
  "General" tab. Basically, what this does is that it let's 
  you control how web pages that want to open new windows 
  on their own (without your permission) should be handled.

  The default is to tell all those porn pages that you have
  been trying to visit (shame on you!) to open in the same window.

  The other two options are to block all such badly behaving 
  .php links or to open them in a completely new CD browser 
  instance window.

  Actually, you could not even visit any such naughty pages 
  before because I had not implemented any open new window
  functionality before 1.5.3 version and CD has not yet tabbed
  browsing support (but will be!).

- Finished context menu handling. Now, when you right click
  any link you can use the following.

  For links:

  * Open Link. This is the same as default behaviour when
    you click any link. What happens will depend of the 
    target of the link. If it's .html page then it will
    naturally open it in the same window. Same also for
    images. If the target of the link is any content that
    CD can't natively handle it will give you option to
    download and save it (Yes, CD has finally download
    support! More of that later...)

  * Open Link in New Window. This is the same as above
    except that a new CD browser window instance is 
    created and the target of the link is tried to open
    in there. 
    
    Important note: No matter what the 
    "Open new window action policy for scripts" has been set,
    the *user* can always open new window whenever he/she
    wishes. So this menu option will skip any policies.

  * Save Link. This downloads and saves the target of
    the link. Be it image, .exe file, zip file, etc....

  * Copy Link. This saves the link address to clipboard.

  For images:

  * Open Image. (I should have renamed this to Open Image
    in New Window). Does just that. Creates a new CD browser
    window instance and opens the image in there.

  * Save Image. Downloads and saves the image.

  * Copy Image to Clipboard. Copies image to clipboard that
    you can paste later to say, into word processor.

    For some reason I could not paste into Gimp program ...

  * Copy Image Address. Copies Image link to clipboard.

- Added HTTP pipelining support to "General" tab and put 
  it on by default.

  This is important for those who are surfing with mobile
  network that have higher latency (connection time) than
  other connection types. Should give you some speed boost.

  You can read more at 
  https://en.wikipedia.org/wiki/HTTP_pipelining

  For proxies pipelining is always disabled.

- Optimized little bit of code for speed. This with above
  mentioned HTTP pipelining and the next major improvement
  should give you very good surfing speed.

Major improvements:

- Added new "Cache" tab and disk cache support (Yay!). 
  Now, by default, all the content you are surfing is cached 
  to your disk for speedup.

  By default the cache directory is called "cache" and it
  will be created the first time you start CD to the same
  holder that it was started. Minimum (and default) size
  for cache is 50 MB and max 2048 MB.
  
  Besides enabling cache you can of course disable it or
  always use it (offline mode). In offline mode all the
  content are fetched always from cache and network is
  never used.

  Also, you can naturally clear the cache any time with
  push of the button or set CD to always clear cache at
  exit.

  Tip of the day: Create a ram drive and tell CD to
  store all files there by giving it's path to
  cache dir fiel.

  This gives insane speed boost because RAM memory is
  always faster than disk. As a bonus it's content will
  be sure to removed when you close your computer.
  If that's not what you want then stay with regular
  (but slower) disk cache.
  I might later add some functionality to handle all 
  this automatically. Including saving cache back to
  disk from RAM at exit and back again at startup.
 
- Added new "Downloads" tab. All the files you download
  will appear here as a simple list showing the filename,
  size, progressbar, start/stop button and status.

  Currently only starting and stopping (cancelling)
  downloads are supported. For download pausing and 
  resuming where it was left will have to wait till 
  CD 1.5.4.
   
  Double-clicking will try to open file with default
  application for that file type.

  If you forgot where you downloaded your file you can
  hover mouse over it's filename and it will show
  full file path. I will make it later easier to access
  downloaded files.  

- Added new "Notification" tab. CD has now notification
  support. Basically this will tell user when some task
  has been completed. For now it supports just three
  download related notifications but I will add more later.

  The notification label are small 288 x 94 pixel yellow labels,
  with notification Title about the task completed (or
  in case of error,failed) and any other relevant data.
  
  By default, the notification labels will stay for 10
  seconds visible or untill closed by pushing close putton
  at labels top right corner. 

  If you click anywhere else at the label, it will stop 
  the timeout.

  If you keep the mouse button pressed and drag mouse you
  can change it's position freely to anywhere on the screen.
  
  Next notification labels will appear at the place you
  left last one and the same position will be saved at exit
  and used next time at startup.

  You can also disable showing timeout completely.
  To disable timeout completely give timeout value of zero.

- Added proxy export support. You can now export any proxies
  you managed to get from proxy fetcher & checker to external
  file. File format is standard text file with each line in
  the form of proxy ip:port number.

- Added proxy import support. Same format as above. After
  proxies have been imported from file CD will fetch some
  additional proxies from net, remove any duplicates and
  start checking their status. 

  If you don't have any premade proxy file at hand 
  there is an proxy file called proxies.txt under CD folder 
  that you can use for importing.

  But be warned: That file has over 700 proxies and 
  proxy fetcher & checker will probably get another 100 more!

  So it's over 800 proxies!!! You better have a *very*
  good computer and internet connection (and lot's of threads!!!)
  to check all those proxies for good ones :-)

So there you have it. I will now take some few day break and
start thinking about 1.5.4. 

Very unlikely that it will be finished before Christmas 
but you never know ;-)


P.S:
I don't know if these were obvious or not but you can surf in
full mode view by just moving your mouse cursor between browser screen
and settings, that line between them, it's called splitter. When 
mouse cursor changes to two headed arrow you are at the right location.
Now press mouse button and drag to the right if you want full mode view
browser window or to the left if you want full mode view settings window.

Also, you can reorder setting tabs by just pressing mouse button over them
and dragging to them left or right. Very useful for mega downloaders who
have to chekc Downloads tab time to time. Just move it to be the first tab
at settings window :-)

Unfortunately, tab positions are not saved between startups :-(
Maybe later...

P.S 2:

Checked just how many have downloaded CD...... And I was very surprised :O
2351 downloads in just few weeks!

Big Warm Thank You to Everyone!!!


---------------------------------------
1.5.2:
Minor improvements:

- Reordered Proxy tab layout and tried to make it much more compact,
  logical and pretty.

Major improvements:

- Added Proxy List filtring. With this you can decide what proxies
  are keeped in the list and what are dropped.

  The defaults are to keep all those proxies that have SSL field set
  as "Yes" or empty (meaning that SSL support could not be determined)
  and Anonymous field "Yes" or empty. These are good defaults
  but if you want stricted settings with your next proxy fetch
  you could leave only "SSL Yes" and "Anonoymous Yes" checked.

  This will of course produce less usable proxies.

- Added random proxy hopping based either on preset timer or per
  page request. What this does is that after you have 
  fetcher & checked your list of proxies you can then let CyberDragon
  to randomly hop between those proxies on the list, either on certain
  timeout or always with each page request. 

  For timed proxy hopping, the default timeout is 10 seconds and max
  3600 (1 hour). If this is too frequent be free to increase timeout.
  Most browsers, including CyberDragon, will wait for max of 30 seconds
  to connect. So if you are unlucky and have a lot of slow proxies on 
  your list you might want to increase timeout at least that value.

  Every time a random proxy is selected it will be clearly shown at the
  status message area at the bottom right.

  When you are finished and don't want to use random proxy hopping
  anymore just select "No proxy hopping option". That will also clear
  any previously used proxy.

  In addition to maybe increasing timeout value you should keep the
  following in mind when using this feature:

    - Use this only for normal Internet surfing. Using this feature
      while checking your webmail, doing online banking or any other
      service which needs authentication is a probably a bad idea.

      Most such services won't probably like it at all when they
      see you connecting suddendly from totally different IP-address
      than just few seconds ago. 

    - There is no automatic proxy online checking yet implemented.
      This means that some proxies on your list might go offline at
      some point and currently the random proxy hopping does not
      detect it. For timed proxy hopping you must wait until next
      timeout but for proxy hopping per page request you can force
      proxy switching by pressing F5 or Reload button.

      With almost 100 proxies currently available, you should be very
      unlucky indeed for all the proxies to go offline :-)

    - Sometimes it might seem like that timed proxy hopping is using
      the same proxy all the time. Computer "random" generators are not
      very good at picking truly random numbers.

      Once I got the same proxy three times on the row with 10 second
      timeout and from a list that had about 20 proxies.

    - Some proxies are horribly slow. I am planning to add caching 
      support on next 1.5.3 version and maybe make it always on when using
      random proxy hopping.

---------------------------------------
1.5.1:
Minor improvements:

- Final proxy results are now sorted so that those proxies with "SSL"
  and "Anonymous" fields both set as "Yes" will appear first in the list.

- Added optional proxy color coding. If the "SSL" and "Anonymous" fields
  are both "Yes" then it means that it is a very good proxy and it is
  colored green. If "Anonymous" field is "No" then proxy is not
  recommended and it is colored red. Everything else is colored with
  yellow to sign as a warning that proxy checker could not for some
  reason determine either SSL or Anonymous support. 

Major improvements:

- Fixed a bad bug with Get proxy button. If you did a proxy fetching
  and after letting it finish you fetched again it would print
  ""Stopping proxy checking. Waiting for remaining threads to finish ...",
  disable Stop button and then do absolutely nothing.
  
  That was because of incorrect button logic.
  Sometimes it would also crash. That was because the thread deleting
  was in completely wrong place. Both bugs should be now fixed.

- Proxy fetcher is now able to fetch about 100 proxies.

---------------------------------------
1.5:
  I have worker for two days now, almost non-stop with
  this new version. I call this version 1.5 Proxy edition because
  all the major improvements are proxy related.
  But first the minor improvements.

Minor improvements:
- Added new Zoom in and Zoom out toolbar buttons. Old key shortcuts,
  (Ctrl Plus for Zoom in & Ctrl Minus for Zoom Out) naturally
  continue to work.

- Added new Printer toolbar button. Key shortcut for it is Ctrl + P.

  Unfortunately, there is a little bug that is related to Qt 
  (https://bugreports.qt-project.org/browse/QTBUG-30621) and 
  that I cannot do nothing about untill a new version of Qt appears
  at the end of November or start of December 2013.

  The effect of this little printing bug is that the first page
  layout is broken and the printed text might be cutted out or
  otherwise look funny or weird.

Major improvements:
- Finally made the proxy fetching multithreaded (Yay!).
  Now the main GUI thread won't freeze and you can keep on surfing
  the Net while the proxy fetcher does it's work at background.

  After the fetcher has gotted it's list of proxies it will start
  the actual proxy checking process which is very intense.

  Here's what happens at the background:

  1. Proxy checker threads are started. The default number of proxy
     checker threads is either the number of CPU cores in your 
     computer or 4. Which ever is greater. 
     Naturally, if you feel that it is too much or too little 
     you can change it and the changes will be saved at exit.

     However, the more threads you set the more system resources 
     (CPU load, RAM,network bandwith) CyberDragon will use. 

     Finally at some point, increasing the number of threads
     actually begins to hurt your system performance and makes 
     things slower.

     You have to experiments to see what is best thread number for
     your machine and connection.
  
  2. Each thread takes in its turn one of the proxies from the list
     and tries to ping it to determine connection speed (aka latency). 
     The smaller the latency number the faster that particular 
     proxy will respond and connect. 

     If the ping attempt does not get respond then it either means that
     the proxy is activately blocking (firewall) the ping request or 
     it is offline. 

     In that case an extra step is taken and an actual connection attempt to 
     proxy server is tried to see if it's online.

     If the proxy server still does not answer or the connection timeout
     has been reached then it is assumed that this particular proxy 
     is offline and it will be removed from the list and the thread 
     continues with the next available proxy in the list.
  
  3. After it has been determined that the proxy is alive the proxy checker
     thread will continue doing geoip lookup to see which country it belongs.
     
     This is useful because some proxies can't access some Internet sites 
     depending of the country the proxy is located.

     For example, if the proxy is located in China you could have a hard time
     of accessing YouTube or Facebook.

     If you don't want country lookup it can be disable but it really does 
     not slow the proxy checking process that much.

  4. Next the proxy SSL support is determined. It's very important that
     you use only proxies that support SSL. Even tought there has been
     weaknesses and attacks against SSL, some encryption is still better than
     no encryption. Besides, without proxy that supports SSL you can't
     access any https:// sites like https://startpage.com,
     https://duckduckgo.com or https://encrypted.google.com.

     If the thread can't for some reason (timeout for example) check if the 
     SSL support is on then that field will have empty value.

  5. Lastly the anonymity level of proxy is determined. Most so called
     high-anonymous proxies do what they promise and hide your true IP-address.

     But there is always bad proxies that inject extra http headers on client
     request. Obviously, you should only use proxies that print "Yes" in that
     field.

  So as you can see there is a lot happening behind background. After all the
  proxies have been checked it will print "Finished!" and show a short summary
  of the number of proxies checked, number of proxies online and number of
  proxies offline.

  There is also timeout value that you can play with if you want to try make
  the whole process faster. The default value is 30000 milleseconds (30 secs.)
  for each proxy connection timeout.

  One final very very important note: Always use proxies that are fast
  (low latency), support encryption (SSL set to 'Yes') and are anonymous and
  hide your IP-address (Anonymous field set to 'Yes'). 

  And never, ever(!) use proxy when accessing your webmail, online bank or
  e-store! For these services always use direct connection.

  There is a very good writing by Steve Gibson why even HTTPS proxy might not
  be secure (https://www.grc.com/fingerprints.htm). For everyday surfing
  proxies are ok but for important things (webmail, online bank etc..) direct
  connection is only option.

  Next version will have further proxy stuff and more proxies.
  Don't forget to donate!
    
---------------------------------------
1.4.7:
Minor improvements:
- Further fixes to "Block mixed content".
  Not only do encrypted search engines need to allow
  unencrypted http:// links. Also some webmails 
  (like gmail & yahoo) need at least some of their
  https:// URLs to allow unencrypted content for
  prober working.

  If this is desirable or not I won't speculate.
  
  However, instead of hard coding all these
  (and maybe some future others?) URLs I decided
  to do the same as with "Cookie Manager", mainly
  to allow saving those specific urls
  (or whole domains in search engine cases) that
  need to skip mixed content blocking.

  You can find it at "Encryption" tab.
  As with "Custom Cookie Rules" the rules are 
  regular expressions and it supports adding, 
  removing, enabling and in place editing of
  those rules (just double click to edit). 
  And they will be automatically saved on exit.

- Added yahoo webmail custom cookie rules. Might be
  too lax. Experiment and report if they need changing.

---------------------------------------
1.4.6:
Minor improvements:
- Fixed "Block mixed content". 
  When you use encrypted search engine like 
  https://startpage.com (default), https://duckduckgo.com 
  or https://encrypted.google.com then there is no way we 
  can keep on blocking mixed content, in this case, 
  search results for http:// links. So obviously those
  have to be allowed.

  So, if "Block mixed content" is enabled I made an exception
  for these three search engines. 

---------------------------------------
1.4.5:
Minor improvements:
- Added new tab called "Encryption" and
  experimental mixed content blocking.

  Mixed content is any unencrypted http content
  that is server from https encrypted page.

  That has potential of compromising your privacy
  and security. Blocked by default.
  More stuff to come to this "Encryption" tab later.

---------------------------------------
1.4:
Major improvements:
- Removed all the references that I could find to Visual Studio.
  This should now fix the runtime error at startup.
  Tested with fresh installation of Windows 7 32-bit Ultimate edition.
  Big Thanks go to Softpedia for reporting this bug.
  
  Lesson of the day for me: Stay away from Visual Studio.
  
  If someone still has problems of starting it up please contact me
  at http://www.binarytouch.com/contact.php

---------------------------------------
1.3:
Minor improvements:
- Fixed little bug of automatic http:// handling in URL bar.
- Fixed little bug between URL bar and page content switching.
- google.com custom cookie rule was too lax. Changed it from .* to
  .*SID. This will still allow using gmail.
- Added zoom in (key shortcut Ctrl Plus) and zoom out (Ctrl Minus)'

---------------------------------------
1.2:
Minor improvements:
- Cleaned up and rearranged Cookie control. Removed reduntant URL field from 
  "Cookie sent by server" view.
- URL bar now accepts Enter-key.
- If you forgot to add http:// or https:// from the URL it will now
  automatically prefix http:// as default.

Major improvements:
- Finally added settings saving (Yay!). Any change you made will be
  automatically saved when you exit the browser. And all the saved settings are
  automatically loaded at startup. Settins are saved in config.ini file.
  
- Improved "Custom Cookie Rules" view.
  Now you can add your own domain specific cookie rules that will override
  global cookie settings. 
  
  This way you will have default deny cookie policy and only cookies that:
  A) Match global cookie settings (which are very strict and should be not
     touched)

  or

  B) Match explicit custom cookie rules will be accepted.
  
  Domain, Path and Name-Value pair can be either literal value or
  regular expression.
  
  Example 1. Literal expression:
  
  mail\.google\.com is an literal expression that will match domain
  mail.google.com (you must prefix any dot with \ character, 
  otherwise it can mean any character)
  
  Example2. Regular expression:
  
  .* as a domain name, path name or name-value pair means any value,
  even empty!
  
  . means any character and * does mean match 0 or more times. 
  So it's literally: "match any character 0 or more times" !
  If you need to match 1 or more times then use +
  
  So a+ means "match a character 1 or more times".
  
  I have added three rules for gmail as an example. They can be
  edited in place and you can add as many other rules as you like. 
    
  For more information about regular expressions take a look at:
  http://perldoc.perl.org/perlre.html
    
  
- Added new "Total Cookies" view. 
  Unlike "Cookie sent by server" view (which shows only 
  Allowed/Blocked cookies per request), this view will show *all* 
  currently collected cookies, both denied and allowed 
  (which are stored into memory)

  This is useful and will help you debugging if you want to add
  your own custom cookie rules.
  
  The idea is that when you are using your favorite webmail, social
  network or whatever, only very minimal cookies that are needed for
  functionality are allowed and everything else is blocked.
  
  When making your custom cookie rules you should check the Total cookies
  for the domain you are currently visiting. Usually those cookies which 
  are session (have no expiration time), and have Secure attribute set 
  as 'Yes' (cookie is only sent via HTTPS) are good candidates for
  allowing and adding to Custom Cookie rule.

  Just click the cookie you want from the list and then click plus (a.k.a.
  "Add custom cookie rule") button and your new cookie rule is added.
  For removal, select it and click minus ("Remove custom cookie rule")
  button.
  To edit cookie rule just double-click the field you want to edit.
  To temporarily disable cookie rule uncheck the checkbox in front of rule.
  
  Don't forget to send me your custom cookie rules so that I can add them
  to future versions.

---------------------------------------
1.1:
Minor UI improvements:
- Added reload button
- Added tooltips for back, home, reload, forward & encrypted button.
- Added Backspace key shortcut to go back in history.
- Added Shift + Backspace key shortcut to go forward in history.
- Added F5 key shortcut for reload.
- Added F6 key shortcut for easy switching between URL bar and web page
  content.
- Made URL bar size longer.
- Shows link now on statusbar when mouse over.

---------------------------------------
1.0:	

First version.
