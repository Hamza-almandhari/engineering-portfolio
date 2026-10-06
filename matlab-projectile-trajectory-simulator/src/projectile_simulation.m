% Get input parameters from user
    disp('Enter the launch parameters:');
    x0 = input('Initial x-position (m): ');
    y0 = input('Initial y-position (m): ');
    D = input('Distance to building (m): ');
    H = input('Building height (m): ');
    W = input('Building width (m): ');
    
    % Fixed target position (10m beyond building, 5m above ground)
    target_x = x0 + D + W + 10;
    target_y = 5;
    
    % Physics constants
    g = 9.81; % gravity (m/s^2)
    
    % Try different angles to find a solution
    found_solution = false;
    for angle = 1:89
        theta = angle; % Launch angle in degrees
        
        % Calculate required time to reach target_x
        delta_x = target_x - x0;
        v0_x = delta_x / (sqrt( (2*(delta_x * tand(theta) - (target_y - y0))) / g ) );
        
        % Calculate required velocity components
        v0 = v0_x / cosd(theta);
        
        % Check if this clears the building (now checking entire width)
        time_to_building_start = D / (v0 * cosd(theta));
        time_to_building_end = (D + W) / (v0 * cosd(theta));
        
        % Check height throughout building width
        clear_building = true;
        for t_check = linspace(time_to_building_start, time_to_building_end, 20)
            y_at_check = y0 + v0 * sind(theta)*t_check - 0.5*g*t_check^2;
            if y_at_check <= H + 0.1  % Small clearance margin
                clear_building = false;
                break;
            end
        end
        
        if clear_building
            found_solution = true;
            break;
        end
    end
    
    % Display results
    if found_solution
        fprintf('\nSolution found:\n');
        fprintf('Launch velocity: %.2f m/s\n', v0);
        fprintf('Launch angle: %.2f degrees\n', theta);
        
        % Calculate and plot trajectory
        flight_time = (delta_x) / (v0 * cosd(theta));
        t = linspace(0, flight_time, 100);
        x = x0 + v0 * cosd(theta) * t;
        y = y0 + v0 * sind(theta) * t - 0.5 * g * t.^2;
        
        % Create plot
        figure;
        hold on;
        grid on;
        
        % Plot building (now with width)
        building_x = [x0+D, x0+D, x0+D+W, x0+D+W];
        building_y = [0, H, H, 0];
        plot(building_x, building_y, 'k-', 'LineWidth', 3);
        fill(building_x, building_y, [0.8 0.8 0.8]);
        
        % Plot trajectory
        plot(x, y, 'b-', 'LineWidth', 2);
        
        % Plot points
        plot(x0, y0, 'ro', 'MarkerSize', 8, 'MarkerFaceColor', 'r');
        plot(target_x, target_y, 'go', 'MarkerSize', 8, 'MarkerFaceColor', 'g');
        
        % Labels
        xlabel('Horizontal Distance (m)');
        ylabel('Height (m)');
        title(sprintf('Projectile Trajectory (%.1f m/s at %.1f°)', v0, theta));
        legend('Building', '', 'Trajectory', 'Launch', 'Target');
        
        % Set axis limits
        xlim([min(x0, target_x)-5 max(x0, target_x)+5]);
        ylim([0 max([y0, max(y), H, target_y])+5]);
        
    else
        fprintf('\nNo solution found with these parameters.\n');
        fprintf('Try increasing the initial height or decreasing the building height/width.\n');
    end