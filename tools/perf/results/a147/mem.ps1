$o = Get-CimInstance Win32_OperatingSystem
'{0:N1} GB in use of {1:N1}' -f (($o.TotalVisibleMemorySize - $o.FreePhysicalMemory) / 1MB), ($o.TotalVisibleMemorySize / 1MB)
