source ~/.bashrc
TIME=10

BELL_EMOJI=$'\U0001F514'
TASK_EMOJI=$'\U0001F4CB'
LINK_EMOJI=$'\U0001F517'
BRANCH_EMOJI=$'\U0001F33F'
if [[ "$CI_JOB_STATUS" == "success" ]]; then
    STATUS=$'\U00002705'
else
    STATUS=$'\U0000274C'
fi

URL="https://api.telegram.org/bot$TELEGRAM_BOT_TOKEN/sendMessage"
# TEXT="$CI_JOB_NAME status: $CI_JOB_STATUS%0A%0AProject:+$CI_PROJECT_NAME%0AURL:+$CI_PROJECT_URL/pipelines/$CI_PIPELINE_ID/%0ABranch:+$CI_COMMIT_REF_SLUG"
TEXT="$BELL_EMOJI *Job*: $CI_JOB_NAME $STATUS%0A%0A$TASK_EMOJI *Project*:+$CI_PROJECT_NAME%0A$LINK_EMOJI *URL*:+$CI_PROJECT_URL/pipelines/$CI_PIPELINE_ID/%0A$BRANCH_EMOJI *Branch*:+$CI_COMMIT_REF_SLUG"

curl -s --max-time $TIME -d "chat_id=$TELEGRAM_USER_ID&disable_web_page_preview=1&parse_mode=markdown&text=$TEXT" $URL > /dev/null