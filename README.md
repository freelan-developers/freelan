# FreeLAN

## What is FreeLAN?

A peer-to-peer, secure, easy-to-setup, multi-platform, open-source,
highly-configurable VPN software.

## Installation

FreeLAN is available on Windows, Mac OSX and Linux (Debian-based distributions).
Check the [official website](http://www.freelan.org) for installers.

If your platform is not officially supported, do not despair and just try to
build it yourself ! Check the [build instructions](BUILD.md) for details on how
to do that.

## Quick-start

FreeLAN can be configured to support any network topology, be it a traditional
client-server structure or a full-mesh one. The emulated network can operate at
the Ethernet or at the IPv4/IPv6 levels. Security can be achieved through the
use of certificates or via a simple shared passphrase.

For instance, creating a simple VPN node can be done with:

    freelan --security.passphrase "my secret"

This will start a VPN node, listening for connections on all interfaces on the
port UDP:12000. The node will pick the default 9.0.0.1 IPv4 address inside the
emulated network.

To connect to another node, just specify the `fscp.contact` option, like so:

    freelan --security.passphrase "my secret" --fscp.contact 1.2.3.4:12000

Each node should have a different virtual IP address: you can specify the IPv4
address to use with the `tap_adapter.ipv4_address_prefix_length` option, like
so:

    freelan --security.passphrase "my secret" --tap_adapter.ipv4_address_prefix_length 9.0.0.2/24

Check out the [configuration file](apps/freelan/config/freelan.cfg) for details
!

## Frequently Asked Questions

### Is there a graphical interface ?

A graphical user interface is planned but nothing is made public yet. You will
just have to be patient :)

### Can I use FreeLAN to access forbidden websites/websites in other countries with restrictions ?

FreeLAN is not a service, it's a software. You can configure it to reach that
goal easily, but you'll need an exit-point (a server, a friendly-computer at the
    right location) on which you can run FreeLAN too. **It will always take a
minimum of 2 nodes to create a VPN network**.

## Donations

You like FreeLAN and you would like to show your support and help development of
next releases ? Check out the [donate page](http://freelan.org/donate.html#) !

## Contributions

Check the [contributions guidelines](CONTRIBUTING.md).

## Packaging

Check the [packaging instructions](packaging/README.md).
