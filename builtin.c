#include "shell.h"

int built_echo(t_parser *cmd)
{
    int i = 1;
    int newline = 1;
    if(cmd->argv[i] && ft_strcmp(cmd->argv[i],"-n") == 0)
    {
        newline = 0;
        i++; 
    }
    while(cmd->argv[i])
    {
        printf("%s",cmd->argv[i]);
        if(cmd->argv[i + 1] != NULL)
            printf(" ");
        i++;
    }
    if(newline)
        printf("\n");
    return 0;
}

int built_cd(t_parser *cmd, t_env **env_list)
{
    char *oldpwd;
    char *pwd;
    char cwd[1024];
    
    // Get current directory before changing
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return 1;
    }
    
    oldpwd = ft_strdup(cwd);
    
    // If no argument, cd to HOME
    if (cmd->argv[1] == NULL) 
    {
        pwd = get_env_value(*env_list, "HOME");
        if (!pwd)
        {
            fprintf(stderr, "cd: HOME not set\n");
            free(oldpwd);
            return 1;
        }
    }
    else
    {
        pwd = cmd->argv[1];
    }
    
    if (chdir(pwd) != 0)
    {
        perror("cd");
        free(oldpwd);
        return 1;
    }
    
    // Update PWD and OLDPWD environment variables
    set_env_value(env_list, "OLDPWD", oldpwd);
    
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        set_env_value(env_list, "PWD", cwd);
    
    free(oldpwd);
    return 0;
}

int builtin_pwd(void)
{
    char cwd[1024];
    if(getcwd(cwd,sizeof(cwd)) != NULL)
    {
        printf("%s\n",cwd);
        return 0;
    }
    else
    {
        perror("pwd");
        return 1;
    }
}

int builtin_exit(t_parser *cmd)
{
    int code = 0;
    if(cmd->argv[1] != NULL)
        code = atoi(cmd->argv[1]);
    printf("exit\n");
    exit(code);
    return 0;
}

int builtin_export(t_parser *cmd, t_env **env_list)
{
    if (!cmd->argv[1]) // No arguments, Hepsini göster
    {
        t_env *current = *env_list;
        while (current)
        {
            printf("declare -x %s=\"%s\"\n", current->key, current->value);
            current = current->next;
        }
        return 0;
    }

    int i = 1;
    while (cmd->argv[i])
    {
        char *arg = cmd->argv[i];
        char *eq_pos = ft_strchr(arg, '=');
        int valid = 1;
        int j = 0;

        // POSIX uyumlu kontrol
        if (!arg[0] || (!((arg[0] >= 'a' && arg[0] <= 'z') || 
                          (arg[0] >= 'A' && arg[0] <= 'Z') || 
                          arg[0] == '_')))
        {
            valid = 0;
        }
        else
        {
            if (eq_pos)
            {
                while (j < (eq_pos - arg))
                {
                    char c = arg[j];
                    if (!((c >= 'a' && c <= 'z') || 
                          (c >= 'A' && c <= 'Z') || 
                          (c >= '0' && c <= '9') || 
                          c == '_'))
                    {
                        valid = 0;
                        break;
                    }
                    j++;
                }
            }
            else
            {
                while (arg[j])
                {
                    char c = arg[j];
                    if (!((c >= 'a' && c <= 'z') || 
                          (c >= 'A' && c <= 'Z') || 
                          (c >= '0' && c <= '9') || 
                          c == '_'))
                    {
                        valid = 0;
                        break;
                    }
                    j++;
                }
            }
        }

        if (!valid)
        {
            printf("export: `%s': not a valid identifier\n", arg);
            i++;
            continue;
        }

        if (eq_pos) // Variable assignment: VAR=value
        {
            *eq_pos = '\0'; // Split key and value
            set_env_value(env_list, arg, eq_pos + 1);
            *eq_pos = '='; // Restore original string
        }
        else // Just export existing variable or create empty one
        {
            t_env *existing = find_env(*env_list, arg);
            if (!existing)
                set_env_value(env_list, arg, "");
        }
        i++;
    }
    return 0;
}

int builtin_unset(t_parser *cmd, t_env **env_list)
{
    if (!cmd->argv[1])
        return 0;
    
    int i = 1;
    while (cmd->argv[i])
    {
        char *var_name = cmd->argv[i];
        int valid = 1;
        int j = 0;
        // POSIX: identifier must start with letter or '_', rest can be letter/digit/_
        if (!var_name[0] || (!((var_name[0] >= 'a' && var_name[0] <= 'z') || (var_name[0] >= 'A' && var_name[0] <= 'Z') || var_name[0] == '_')))
            valid = 0;
        else {
            while (var_name[j]) {
                char c = var_name[j];
                if (!( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
                    valid = 0;
                    break;
                }
                j++;
            }
        }
        if (!valid) {
            printf("unset: `%s': not a valid identifier\n", var_name);
            i++;
            continue;
        }
        unset_env_value(env_list, var_name);
        i++;
    }
    return 0;
}

int builtin_env(t_parser *cmd, t_env *env_list)
{
    (void)cmd;
    t_env *current = env_list;
    while (current)
    {
        printf("%s=%s\n", current->key, current->value);
        current = current->next;
    }
    return 0;
}