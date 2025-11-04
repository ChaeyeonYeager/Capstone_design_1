import subprocess
cmd = 'powershell "Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match \'Camera\' } | Select-Object -ExpandProperty Name"'
print(subprocess.run(cmd, capture_output=True, text=True, shell=True).stdout)