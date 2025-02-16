
function Show-Calendar {
    param (
        [int]$Month = (Get-Date).Month,
        [int]$Year = (Get-Date).Year
    )
    
    $firstDay = Get-Date -Year $Year -Month $Month -Day 1
    $daysInMonth = [DateTime]::DaysInMonth($Year, $Month)
    $monthName = (Get-Culture).DateTimeFormat.GetMonthName($Month)
    $today = Get-Date
    
    # Print header
    Write-Host "`n $monthName $Year" -ForegroundColor Yellow
    Write-Host "Su Mo Tu We Th Fr Sa" -ForegroundColor Cyan
    
    # Print leading spaces
    $offset = [int]$firstDay.DayOfWeek
    Write-Host -NoNewline (" " * 3 * $offset)
    
    # Print days
    for ($day = 1; $day -le $daysInMonth; $day++) {
        # Check if this is today's date
        if ($day -eq $today.Day -and $Month -eq $today.Month -and $Year -eq $today.Year) {
            Write-Host ($day.ToString("00")) -NoNewline -ForegroundColor Green -BackgroundColor DarkGray
        } else {
            Write-Host ($day.ToString("00")) -NoNewline
        }
        Write-Host " " -NoNewline
        if (([int]$firstDay.DayOfWeek + $day - 1) % 7 -eq 6) {
            Write-Host ""
        }
    }
    Write-Host "`n"
}
