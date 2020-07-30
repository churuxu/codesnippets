
#服务脚本模板 
#需外部定义 NAME BINARY ARGS BOOTSTRAP BINARY_DIR WORK_DIR

APP_NAME=${NAME} 

#程序完整路径
APP_PATH=${BINARY_DIR}/${BINARY}

#程序参数
APP_ARGS="${ARGS}"

#PID文件
PID_FILE=/var/run/${APP_NAME}.pid

#启动命令
APP_PREFIX="${BOOTSTRAP}"



show_ok(){
	echo [  OK  ]
    echo 	
}


show_fail(){
	echo [FAILED]
    echo 	
}



#直接运行
run(){
	if [ -f ${PID_FILE} ]; then
		PID=$(cat ${PID_FILE})
		if [ -d /proc/${PID} ]; then
			CNT=$(cat /proc/${PID}/cmdline | grep ${APP_PATH} | wc -l)
			if [ ! $CNT -eq 0 ]; then				
				kill $PID > /dev/null 2>&1
				rm -f ${PID_FILE}
			fi        
		fi
    fi
	
	if [ ! "${WORK_DIR}" = "" ]; then
		[ ! -d ${WORK_DIR} ] && mkdir -p ${WORK_DIR}
		cd ${WORK_DIR}
	else
		cd ${BINARY_DIR}
	fi
	
	chmod +x ${APP_PATH}
	${APP_PREFIX} ${APP_PATH} ${APP_ARGS} 	
}


#启动 
start(){
	if [ -f ${PID_FILE} ]; then
		PID=$(cat ${PID_FILE})
		if [ -d /proc/${PID} ]; then
			CNT=$(cat /proc/${PID}/cmdline | grep ${APP_PATH} | wc -l)
			if [ ! $CNT -eq 0 ]; then
				echo "$APP_NAME is running"
				return 1
			fi        
		fi
    fi

	echo -n "Starting $APP_NAME: "
	
	if [ ! "${WORK_DIR}" = "" ]; then
		[ ! -d ${WORK_DIR} ] && mkdir -p ${WORK_DIR}
		cd ${WORK_DIR}
	else
		cd ${BINARY_DIR}
	fi
	
	if [ ! -f ${APP_PATH} ]; then
        show_fail
        return 1
	fi	
	
	if [ -f ${BINARY_DIR}/prestart.sh ]; then		
        chmod +x ${BINARY_DIR}/prestart.sh
		${BINARY_DIR}/prestart.sh || return 1        
	fi		
	
	chmod +x ${APP_PATH}
	nohup ${APP_PREFIX} ${APP_PATH} ${APP_ARGS} > /dev/null 2>&1 &	
	PID=$!
	if [ $PID -eq 0 ]; then
        show_fail
        return 1
	fi	
	#echo "pid: ${PID} ${OK}"
	sleep 1
	if [ ! -d /proc/${PID} ]; then
        show_fail
        return 1
    fi	
	
	echo ${PID}>${PID_FILE}
	show_ok
	return 0
}

# 停止
stop(){
    echo -n  "Stopping $APP_NAME: "
	if [ ! -f ${PID_FILE} ]; then
        echo $"$APP_NAME is stoped"
        return 1
    fi
    PID=$(cat ${PID_FILE}) 
	rm -f ${PID_FILE}
    kill $PID > /dev/null 2>&1
    show_ok    
    return 0
}

#重启
restart(){
    stop
	sleep 1
    start
    return $?
}

#查看状态
status(){
	if [ -f ${PID_FILE} ]; then
		PID=$(cat ${PID_FILE})
		if [ -d /proc/${PID} ]; then
			CNT=$(cat /proc/${PID}/cmdline | grep ${APP_PATH} | wc -l)
			if [ ! $CNT -eq 0 ]; then
				echo "$APP_NAME is running"
				return 0
			fi
		fi
    fi
    echo "$APP_NAME is stoped"
    return 1    
}




case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  status)
    status
    ;;
  restart)
    restart
    ;;
  run)
    run
    ;;	
  *)
    echo "Usage: $0 {start|stop|status|restart|run}"
    exit 1
esac

exit $?