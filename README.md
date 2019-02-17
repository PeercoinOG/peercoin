
PeercoinOG Official Development Repo
==================================


This client is compatible with the Peercoin network as long as SegWit is not activated. If SegWit goes live on the network this client will be updated with a checkpoint that will reject the SegWit fork. We hope that forking won't be necessary due to lack of support for SegWit, but we must prepare for the worst. If push comes to shove we hope most users will gather behind PeercoinOG and a Segwit free chain.

This is how you can build in Terminal on Debian based Linux
---------------------

	git clone https://github.com/PeercoinOG/peercoin
	cd peercoin
	git checkout v0.7.1ppc
	./contrib/vagrant/install.sh

References
---------------------
* [Project Website](https://peercoin.net)
* [Whitepaper](https://peercoin.net/whitepaper)
* [Discuss on Reddit](https://www.reddit.com/r/PeercoinOG)
* [Discuss on Forum](https://talk.peercoin.net)
