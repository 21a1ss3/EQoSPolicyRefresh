# Enterprise QoS (EQoS, QoS) policy updater utility

This is very small tiny project which came out of few hours of debugging failure of QoS policies application on Windows. Purpose of current executable is to isolate QoS policies application process to allow it easily been monitored with tools like Process Monitor. But overall it may be used in some other cases, to manually trigger EQoS/QoS policy application.

## EQoS architecture

```
+------------------+          +----------------------+
|                  |          |                      |
|   Group policy   |          |      Powershell      |
|   (gpedit.msc)   |          |  (New-NetQosPolicy)  |
|                  |          |                      |
+------------------+          +----------------------+
         |                                |
         |                                |
         |                                |
         +--------------+-----------------+
                        | 
                        |
                        |
                       \|/
                +----------------+
                |                |
                |  Registry.pol  |
                |                |
                +----------------+
                        |
                        |
                        |
                       \|/
                +----------------+
                |                |
                |  gpupdate.exe  |
                |                |
                +----------------+
                        |
                        |
                        |
                       \|/
                 +--------------+
                 |              |
                 |  gptext.dll  |
                 |              |
                 +--------------+
                        |
                        |
                        |
                       \|/
                 +-------------+
                 |             |
                 |  tcpip.sys  |
                 |             |
                 +-------------+
```

After some tome of digging dows the picture shown in the block above arisied. Whenever you does configuration through `gpedit.msc` or Powershell' cmdlet `New-NetQosPolicy` (without `-PolicyStore ActiveStore` attribute) your values first end up in Group policy registry `Registry.pol` file and then get applied through regular group policy logic. During that process QoS (EQoS) policies finally get written to `HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows\QoS` and then `tcpip.sys` get triggered to refetch them through `NtDeviceIoControlFile` function.

This program runs that last portion of policy application around `NtDeviceIoControlFile` allowing you separate this process and also launch all necessary loggin tools to collect usefull information.

Meanwgile `New-NetQosPolicy -PolicyStore ActiveStore` seems to directly communicating with `tcpip.sys` and doesn't leave records in `Registry.pol` (what make sense). Same difference applies for `Get-NetQosPolicy` cmdlet.

## Toolkit which you may consider to use
This is suggestion set, feel free to change tools at your disgrace
- Process Monitor: Add highighlt filter rule for `Path` `contains` `QoS_Refetcher_`. Program attempts to open dummy files for such purpose of being visible in ProcMon record. After applying such rule you can scroll down till you see highlighted area and quickly sklip uninteresting noise of executable getting loaded
- Perfom: You can use it to collect event from etw source `Microsoft-Windows-EQoS` `{54CB22FF-26B4-4393-A8C2-6B0715912C5F}` (source: https://hannahsuarez.github.io/2019/List-of-ETW-Providers/ + disassembly of tcpip.sys). You can registry a new user collection for events and choose this provider. Later on collected events can be open with reguler Event Viewer
- Regedit: look what's going on in the registry
- Hex Editor: Allows you to clear some junk from `Registry.pol` which could be unreachable by other means. `Registry.pol` has very simple file format: https://learn.microsoft.com/en-us/previous-versions/windows/desktop/policy/registry-policy-file-format
- Utility from this repo: run it in order

## Symptoms

- Qos stopped suddenly work, but was performing well before
- `gpupdate /force` displaying follwoing message: 
```
Windows failed to apply the Enterprise QoS settings. Enterprise QoS settings might have its own log file. Please click on the "More information" link.
User Policy update has completed successfully.

For more detailed information, review the event log or run GPRESULT /H GPReport.html from the command line to access information about Group Policy results.
```
With no further infromation of where to obtain `"More information" link` )
- Windows event with message `Windows failed to apply the Enterprise QoS settings. Enterprise QoS settings might have its own log file. Please click on the "More information" link.`, EventId `1085`, Provider `Microsoft-Windows-GroupPolicy` (`{aea1b4fa-97d1-45f2-a64c-4d69fffd92c9}`).

## TLDR

This is very simple utility with little funconality, but hopefully overview of the process helps you understand nature of what's going on underneath and how to approach it) You also may use that utility for non-debugiing purposes, if you want to only apply QoS policies at given time, but manually contructing them and locating in the registry.