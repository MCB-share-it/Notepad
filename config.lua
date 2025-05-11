config = {
    background_opacity = 0.85, 
    glow_intensity = 0.3,       
    blink_speed = 500,          
    font_size = 14,            
    font_family = "Consolas",   
    window_title = "Super "
}

-- Terminal state
terminal = {
    text = "",
    cursor_visible = true,
    last_blink = os.time() * 1000,
}

-- Create the main window
function create_terminal()
    -- Set up the window properties
    set_window_size(1000, 1000)
    set_window_title("Terminal Effect")
    
    -- Configure background
    set_background_color(0, 0, 0, config.background_opacity * 255)
    enable_transparency(true)
end

-- Update cursor visibility
function update_cursor()
    local current_time = os.time() * 1000
    if current_time - terminal.last_blink >= config.blink_speed then
        terminal.cursor_visible = not terminal.cursor_visible
        terminal.last_blink = current_time
    end
end

-- Draw the terminal interface
function draw_interface()
    -- Clear screen
    clear_screen()
    
    -- Draw background with subtle gradient
    draw_rect(0, 0, get_window_width(), get_window_height(),
              20, 20, 20, config.background_opacity * 255)
    
    -- Draw text with glow effect
    set_font(config.font_family, config.font_size)
    set_text_color(0, 255, 0)  -- Classic green terminal color
    
    -- Add glow effect
    for i = -2, 2 do
        for j = -2, 2 do
            if i == 0 and j == 0 then goto skip end
            draw_text(
                20 + i, 
                40 + j, 
                terminal.text,
                false
            )
        end
        ::skip::
    end
    
    -- Draw actual text
    draw_text(20, 40, terminal.text, false)
    
    -- Draw blinking cursor
    if terminal.cursor_visible then
        draw_text(20 + #terminal.text * 10, 40, "_", false)
    end
end

-- Handle keyboard input
function handle_input(key)
    if key == "\b" then
        terminal.text = terminal.text:sub(1, -2)
    elseif key ~= "" then
        terminal.text = terminal.text .. key
    end
end

-- Main update loop
function update()
    update_cursor()
    draw_interface()
end

-- Initialize terminal
function init()
    create_terminal()
    -- Initial prompt
    terminal.text = "$ "
end

-- Register functions
return {
    init = init,
    update = update,
    handle_input = handle_input
}
