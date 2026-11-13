# Lane A58: type a console command (or one key) into a game window by posting window messages. Works while another window
# (here a system "Windows Security" dialog) holds the foreground, where SetForegroundWindow and SendKeys cannot.
#   post.ps1 -ProcessId <pid> -Key Enter          one Return (the film-slate menu's PLAY)
#   post.ps1 -ProcessId <pid> -Text "stat unit"   Tilde opens the console; each character goes as WM_KEYDOWN (+ shift) / WM_CHAR / WM_KEYUP; Return runs it
param([int]$ProcessId, [string]$Text = '', [string]$Key = '', [int]$Mode = 1)
Add-Type @"
using System; using System.Runtime.InteropServices;
public static class A58P {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern short VkKeyScan(char c);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint code, uint type);
  static IntPtr L(uint scan, bool up) { uint v = 1u | (scan << 16); if (up) v |= 0xC0000000u; return (IntPtr)(long)v; }
  public static void Key(IntPtr h, int vk, int ch) {
    uint scan = MapVirtualKey((uint)vk, 0);
    PostMessage(h, 0x0100, (IntPtr)vk, L(scan, false));
    if (ch != 0) PostMessage(h, 0x0102, (IntPtr)ch, L(scan, false));
    System.Threading.Thread.Sleep(25);
    PostMessage(h, 0x0101, (IntPtr)vk, L(scan, true)); System.Threading.Thread.Sleep(15); }
  public static void Type(IntPtr h, string s, int mode) {
    foreach (char c in s) {
      if (mode == 0) { PostMessage(h, 0x0102, (IntPtr)c, L(0, false)); System.Threading.Thread.Sleep(15); continue; }
      short k = VkKeyScan(c); int vk = k & 0xFF; bool shift = (k & 0x100) != 0;
      if (shift) PostMessage(h, 0x0100, (IntPtr)0x10, L(0x2A, false));
      Key(h, vk, c);
      if (shift) PostMessage(h, 0x0101, (IntPtr)0x10, L(0x2A, true));
    } }
}
"@
$h = (Get-Process -Id $ProcessId).MainWindowHandle
if ($Key -eq 'Enter') { [A58P]::Key($h, 0x0D, 13); exit 0 }
if ($Key -eq 'Esc') { [A58P]::Key($h, 0x1B, 27); exit 0 }
[A58P]::Key($h, 0xC0, 0); Start-Sleep -Milliseconds 400
[A58P]::Type($h, $Text, $Mode); Start-Sleep -Milliseconds 200
[A58P]::Key($h, 0x0D, 13)
