BUILD_DIR = File.join(__dir__, 'build', 'debug')
PWD = File.expand_path(__dir__)
PID_FILE = File.join(BUILD_DIR, 'raptor.pid')

directory BUILD_DIR do
  sh "cmake -S #{PWD} -B #{BUILD_DIR} -G Ninja -DCMAKE_BUILD_TYPE=Debug"
end

desc "Compile the project"
task compile: BUILD_DIR do
  sh "cmake --build #{BUILD_DIR} --parallel"
end

desc "Run the project"
task run: :compile do
  pid = Process.spawn(File.join(BUILD_DIR, 'Raptor'), chdir: BUILD_DIR)
  File.write(PID_FILE, pid)
  puts "Started process #{pid} with PID file at #{PID_FILE}"
  Process.wait(pid)
end

desc "Watch for changes and recompile"
task :watch do
  require 'listen'
  listener = Listen.to('src', 'include') do |_modified, _added, _removed|
    Rake::Task[:compile].execute
    if File.exist?(PID_FILE)
      pid = File.read(PID_FILE).to_i
      begin
        Process.kill('SIGHUP', pid)
      rescue Errno::ESRCH
        puts "Process #{pid} not found"
      end
    end
  end
  puts "Listening for changes..."
  listener.start
  sleep
end

namespace :headers do
  desc "Update header files"
  task :update do
    {
      "include/pico_ecs.h" => "https://raw.githubusercontent.com/empyreanx/pico_headers/refs/heads/main/pico_ecs.h",
    }.each do |local_path, url|
      sh "curl -o #{local_path} #{url}"
    end
  end
end

task default: :run
