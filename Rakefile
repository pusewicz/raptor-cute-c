RELOADABLE = ENV.fetch('RELOADABLE', "ON").upcase
BUILD_TYPE = ENV.fetch('BUILD_TYPE', 'Debug')
BUILD_DIR = File.join(__dir__, '.build', "#{BUILD_TYPE}-#{RELOADABLE == "ON" ? "reloadable" : "static"}")
PWD = File.expand_path(__dir__)
PID_FILE = File.join(BUILD_DIR, 'raptor.pid')
EXE_FILE = File.join(BUILD_DIR, 'Raptor')

directory BUILD_DIR do
  flags = %W[
    -S#{PWD}
    -B#{BUILD_DIR}
    -GNinja
    -DCMAKE_BUILD_TYPE=#{BUILD_TYPE}
    -DRELOADABLE=#{RELOADABLE}
  ]
  sh "cmake #{flags.join(' ')}"
end

desc 'Build the project'
task build: BUILD_DIR do
  sh "cmake --build #{BUILD_DIR} --parallel"
end

def run(cmd)
  puts "Executing: #{cmd}"
  pid = Process.spawn(cmd, chdir: BUILD_DIR)
  File.write(PID_FILE, pid)
  puts "Started process #{pid} with PID file at #{PID_FILE}"
ensure
  Process.wait(pid)
  File.delete(PID_FILE) if File.exist?(PID_FILE)
end

desc 'Run the project'
task run: :build do
  run EXE_FILE
end

namespace :run do
  desc 'Run the project with debugger attached'
  task :debugger do
    run "lldb #{EXE_FILE} -o run"
  end
end

desc 'Watch for changes and rebuild'
task :watch do
  require 'listen'
  listener = Listen.to('src', 'include') do |_modified, _added, _removed|
    if File.exist?(PID_FILE)
      pid = File.read(PID_FILE).to_i

      begin
        Process.kill(0, pid)
        Rake::Task[:build].execute
        puts "Sending SIGHUP to process #{pid}"
        Process.kill('SIGHUP', pid)
      rescue Errno::ESRCH
        warn "Process #{pid} not found"
      end
    else
      warn "No PID file found at #{PID_FILE}, not sending SIGHUP"
    end
  end
  puts 'Listening for changes...'
  listener.start
  sleep
end

namespace :headers do
  desc 'Update header files'
  task :update do
    {
      'include/pico_ecs.h' => 'https://raw.githubusercontent.com/empyreanx/pico_headers/refs/heads/main/pico_ecs.h'
    }.each do |local_path, url|
      sh "curl -o #{local_path} #{url}"
    end
  end
end

desc "Formats the source code using clang-format"
task :format do
  sh %(fd --full-path src/ -e "c" -e "h" | xargs clang-format -i)
end

task default: :build
