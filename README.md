# Network UTilities

Nut manages your network devices (dhcp, static, zeroconf); currently
only supports IPv4 on Linux.

Support for wpa_assistant is included in the client.

Requires:

* Qt5
* resolvconf / openresolv
* libnl (server) >= 1.1
* libiw (client)

Components:

* nuts: Server
* libnutcommon: Library needed from server and client
* libnutclient: Client library
* libnutwireless: Client library for wpa_supplicant
* qnut: Qt Client
* cnut: Command line client

Example config in the source:

* /docs/config.example

## Config syntax

The config consists of nested statements; each statement is terminated
by either a semicolon (`;`) or a block (`{` ... `}`); additional
semicolons at the end of a statement can be added (for example after a
block).

An empty block can always be replaced by a `;`; if a block contains only
a single statement the curly braces can be omitted. Below only the block
variants will be listed.

In the syntax descriptions below:

* `[` ... `]` means that the parts between the square brackets is
  optional.
* `<<` ... `>>` stands for a (possibly empty) set of statements which
  can occur in the given context.
* `<` ... `>` is a placeholder

The global context can only contain `device` statements.

### `device`

Only devices listed in the config are managed; but if they are managed,
they shouldn't be managed by something else - nuts won't cooperate well
with other network managers on the same device.

The `device` statement can only occur in global context.

Syntax:
* `device [ regexp ] <name> { <<device context, environment context>> }`

`<name>` is a string containing the name of the device to be managed. If
`regexp` is specified, the name of the device needs to be exactly
matched by the regular expression in `<name>`; if `regexp` is not
specified but `<name>` contains one of `?`, `[`, `]` or `*` an exact
wildcard match is used.

Environment context statements in the block will apply to the `default`
environment.

Examples:

* `device "usb*" ...` matches `usb`, `usb0` and `usb1` but not `eusb`
* `device regexp "usb.*" ...` - the same as `device "usb*" ...`
* `device "eno1" ...` only matches `eno1`

### `no-auto-start`

This statement can only occur in device context; it tells nuts to not
start a device automatically (which it would by default).

Syntax:
* `no-auto-start;`

For example you might want to enable wireless devices manually to save
power:

```
device "wlp3s0" {
	no-auto-start;
}
```

### `metric`

The `metric` option specifies which device should be preferred when
there are multiple routes for the same target address. For example
you might want to prefer your wired device over the wireless device if
both have a default gateway.

The default metric is `-1`, i.e. no metric is set on the routes; these
routes are always preferred over any other metric. Otherwise routes with
a lower metric are preferred.

The `metric` statement can only occur in device context, but it might
also occur as option after `static`, `dhcp` and `zeroconf` (see those
statements for the syntax), overwriting the environment metric for a
specific address configuration.

Syntax:
* `metric <metric>;`

Example:

```
device "eno1" {
	metric 10;
}

device "wlp3s0" {
	metric 20;
	environment "superfast" {
		dhcp metric 5;
	}
}
```

### `wpa-supplicant`

When `wpa-supplicant` is configured for a device, nuts starts the
wpa_supplicant when a device gets activated (and stops it on turning it
off).

The `wpa-supplicant` statement can only occur in device context, and
must not be given more than once per device.

Syntax:
* `wpa-supplicant config <configfile> [ driver <driver> ];`
* `wpa-supplicant [ driver <driver> ] config <configfile>;`

`<configfile>` is a string containing the location of the
`wpa_supplicant` config file, `<driver>` defaults to `"wext"` and is
forwarded to `wpa_supplicant` after the `-D` option (`nl80211`, `wext`,
`wired`, ...)

Example:

```
device "wlp3s0" {
	wpa-supplicant config "/etc/wpa_supplicant/wpa_supplicant.conf" driver "wext";
}
```

In order to be able to control the wpa_supplicant as normal user you
probably want to have something like this in your `wpa_supplicant.conf`:

```
ctrl_interface=/run/wpa_supplicant
ctrl_interface_group=netdev
update_config=1
```

### `environment`

The environment option can only occur in device context; if an
environment name is given it starts a new environment with that name.

If no name is given it appends to the default environment; these
statements could also simply be given in device context.

For now environment names don't need to be unique, although that might
change in the future. The default environment has an empty name.
(Internally environments are identified by their position in the list;
the default environment is always at index 0.)

By default an environment:
* will use DHCP unless some way of configuring an IPv4 address is given
  or `no-dhcp` is specified
* will be user selectable (`select user;`) unless explicit `select`
  rules are given.

Syntax:
* `environment { <<environment context>> }`
* `environment <name> { <<environment context>> }`

Example:

```
device "eno1" {
	environment {
		dhcp fallback 10 zeroconf;
	}
	environment "static" {
		static user;
	}
}
```

which is the same as:

```
device "eno1" {
	dhcp fallback 10 zeroconf;
	environment "static" static user;
}
```


### `dhcp`

Explicitly enables DHCP in an environment context; can only be given
once per environment.

Syntax:
* `dhcp [metric <metric>];`
* `dhcp [metric <metric>] fallback [<timeout in seconds>] static ...`
* `dhcp [metric <metric>] fallback [<timeout in seconds>] zeroconf ...`
* `dhcp [metric <metric>] fallback { <<fallback context>> }`

`static` and `zeroconf` are parsed like they normally would be in
environment context (but `static user` is not allowed).

In the fallback context one of the `static` or `zeroconf` statements can
be given (otherwise fallback isn't used), also there are two special
statements:

* `timeout <timeout in seconds>;`
* `continue-dhcp true;` or `continue-dhcp false;`

If the timeout is 0 or not specified, `continue-dhcp` is activated by
default and cannot be turned off. For other timeout values `continue-
dhcp` is not enabled by default.

If `continue-dhcp` is enabled, DHCP continues to search for an address
after the timeout has been reached and switches to the DHCP address if it
finds one, otherwise it disables DHCP after reaching the timeout.

### `no-dhcp`

Disables the DHCP default configuration in an environment context; only
needed if no other configuration is specified (like `static` or
`zeroconf`) and you really don't want an IPv4 address.

Syntax:
* `no-dhcp;`

### `static`

Configures a static IP address in an environment context.

Syntax:
* `static [metric <metric>] user;`
* `static [metric <metric>] { <<static context>> }`

`static user` requires the configuration to be entered at runtime by the
user.

In the static context at least the IP address must be configured; the
available statements in static context are:

* `ip <address>;`
* `netmask <address>;`
* `gateway <address>;`
* `dns-server <address>;`

Apart from `dns-server` each statement must not be given more than once.

### `zeroconf`

Configures an IP address in an environment context using using zeroconf
(RFC 3927); can only be given once per environment.

Syntax:
* `zeroconf [metric <metric>];`

### `select`

nuts can select an environment on certain conditions automatically. The
`select` statement must occur in environment context and can only be
given once per environment.

Syntax:
* `select <condition>`

Where `<condition>` can be any of the following:
* `user;` - user can select this environment manually
* `arp <ipv4 address> [<mac address>];` - checks IPv4 address is present
  in local network (and optionally whether it has the expected MAC
  address).
* `essid <essid>;`
* `and { <condition list> }`
* `or { <condition list> }`

Example:

```
device "eno1" {
	environment "home" select or {
		arp 192.168.0.1 00:11:22:33:44:55;
		user;
	}
}

device "wlp3s0" {
	wpa-supplicant config "/etc/wpa_supplicant/wpa_supplicant.conf" driver "wext";
	environment "home" select or {
		essid "my-home-wifi";
		user;
	}
}
```
