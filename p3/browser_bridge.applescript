-- Native browser object references avoid JXA's tab type coercion.
on run argv
    set scriptText to item 1 of argv
    set lastProblem to "Keep the Black box HackerRank page selected in your browser."

    if application "/Applications/Arc.app" is running then
        tell application "/Applications/Arc.app"
            try
                set pageURL to URL of active tab of front window
                if pageURL starts with "https://www.hackerrank.com/" or pageURL starts with "https://hackerrank.com/" then
                    try
                        return execute (active tab of front window) javascript scriptText
                    on error problem number code
                        error "Arc JavaScript execution: " & problem number code
                    end try
                end if
            on error problem number code
                set lastProblem to "Arc: " & problem & " (" & code & ")"
            end try
        end tell
    end if

    if application "/Applications/Safari.app" is running then
        tell application "/Applications/Safari.app"
            try
                set pageURL to URL of current tab of front window
                if pageURL starts with "https://www.hackerrank.com/" or pageURL starts with "https://hackerrank.com/" then
                    try
                        return do JavaScript scriptText in current tab of front window
                    on error problem number code
                        error "Safari JavaScript execution: " & problem number code
                    end try
                end if
            on error problem number code
                set lastProblem to lastProblem & "; Safari: " & problem & " (" & code & ")"
            end try
        end tell
    end if

    error lastProblem
end run
