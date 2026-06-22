# Remote Development Setup with sshfs

This guide explains how to edit code on your local PC and compile on a remote machine using `sshfs` (SSH Filesystem).

## Quick Start

### Step 0: Install sshfs (first time only)
```bash
sudo apt install sshfs
```

### Step 1: Mount remote folder
```bash
~/mount_remote.sh
```
This mounts the remote project to `~/remote-elrepo/`

### Step 2: Edit code locally
```bash
code ~/remote-elrepo
```
Open VS Code or your editor and edit files normally. Changes sync automatically to the remote PC.

### Step 3: Compile on remote (in a separate terminal)
```bash
~/build_remote.sh
```
This compiles the project on the remote machine via SSH.

### Step 4: Unmount when done
```bash
fusermount -u ~/remote-elrepo
```

---

## How It Works

| Your PC | Remote PC |
|---------|-----------|
| Edit in `~/remote-elrepo/` | ← sshfs mount → | Files stored in `/home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/` |
| | ← SSH command → | Compile: `/home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/build/` |

### Example workflow:
1. Edit `~/remote-elrepo/apps/Cluster/qtApp/src/systemInfo.cpp`
2. File instantly appears on remote at `/home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/src/systemInfo.cpp`
3. Run `~/build_remote.sh` to compile everything on remote
4. Get compile output and executable on remote machine

---

## Troubleshooting

### "Permission denied" on mount?
```bash
# Add your user to fuse group
sudo usermod -aG fuse $USER
# Then log out and log back in
```

### "Already mounted" error?
```bash
fusermount -u ~/remote-elrepo
# Then try mounting again
```

### Build fails on remote?
```bash
# SSH directly to check:
ssh team2@100.103.187.55
cd /home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/build
cmake ..
cmake --build .
```

### Connection timeout?
- Check remote PC is online: `ping 100.103.187.55`
- Check SSH works: `ssh team2@100.103.187.55 echo "Connected"`

---

## Commands Reference

| Command | What it does |
|---------|-------------|
| `~/mount_remote.sh` | Mount remote folder to `~/remote-elrepo/` |
| `~/build_remote.sh` | Build project on remote PC |
| `fusermount -u ~/remote-elrepo` | Unmount remote folder |
| `code ~/remote-elrepo` | Edit files in VS Code |

---

## Tips

✅ Use sshfs for **editing**, SSH for **compiling** (faster)  
✅ Keep mount point open while working  
✅ Unmount before disconnecting from network  
✅ Both PCs need same project structure (they should match now)
