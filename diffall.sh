git diff 12a376dcef78c51d27f5b44139b8133dc7cf3af7 HEAD -- . ':(exclude)boot.sh' | \
  sed -E '/^index /d; /^--- /d; /^\+\+\+ /d; s|^diff --git a/.* b/(.*)|\n\1:|' | \
  sed '1{/^$/d}' > tlbopt700.txt
