# Operating limits

Jetlink is experimental. If the link drops or lags while engaged, the comma
says **TAKE CONTROL** and stays engaged on the small model. Be ready to take
over, especially during the first few seconds. See [daily use](using-jetlink.md).

<a id="status-and-known-limitations"></a>
<a id="platform-testing"></a>

## Platform support

Jetson with JetPack 7.2.1, Mac, iPhone, and Ubuntu PCs have been tested.
JetPack 6.2, WSL2, and other supported Linux distributions have not been tested
on hardware with this release. Android performance is not established.

Use the requirements in your [setup guide](README.md#set-up).

<a id="measured-performance"></a>
<a id="what-still-needs-validation"></a>

## Performance

Each frame has a **50 ms budget**, including the cable and comma. Heat can
slow the computer; keep it cooled and check for slow frames. Sustained use at
high temperatures is untested.

On Jetson, prefer the 766 MB models. The 1.7 GB Lebowski leaves less margin.
[Jetson measurements](jetson-performance.md) · [Mac measurements](mac-performance.md)

## Power and connection

Use separate power for the comma and computer. Voltage drops and computer
sleep can drop the link. Keep laptops powered and awake, and the iPhone app
on screen. Connect over USB; TCP is for testing.

[Cables and power](transport.md) · [Troubleshooting](troubleshooting.md)
